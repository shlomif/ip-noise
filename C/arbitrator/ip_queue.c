/*
 * This is a module which is used for queueing IPv4 packets and
 * communicating with userspace via netlink.
 *
 * (C) 2000 James Morris, this code is GPL.
 *
 * Modified by Shlomi Fish to put all the logic inside the kernel.
 *
 * 2000-03-27: Simplified code (thanks to Andi Kleen for clues).
 * 2000-05-20: Fixed notifier problems (following Miguel Freitas' report).
 * 2000-06-19: Fixed so nfmark is copied to metadata (reported by Sebastian 
 *             Zander).
 * 2000-08-01: Added Nick Williams' MAC support.
 *
 */

/*
 * Mental Note by Shlomi Fish and Roy Glasberg: don't look at us for
 * explanations about the original code! We practically ripped it and
 * modified it for our needs.
 * */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/notifier.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/rtnetlink.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <net/sock.h>
#include <net/route.h>

#include <linux/netfilter_ipv4/libipq.h>
#include <linux/netfilter_ipv4/ip_tables.h>

#include "queue.h"
#include "delayer.h"
#include "rand.h"
#include "k_pthread.h"
#include "k_time.h"
#include "k_stdlib.h"
#include "k_stdio.h"

#include "packet_logic.h"

#define IPQ_QMAX_DEFAULT 1024
#define IPQ_PROC_FS_NAME "ip_queue_ker"
#define NET_IPQ_QMAX 2088
#define NET_IPQ_QMAX_NAME "ip_queue_ker_maxlen"

typedef int (*ipq_send_cb_t)(ipq_queue_element_t *e);

typedef struct ipq_peer {
	pid_t pid;			/* PID of userland peer */
	unsigned char died;		/* We think the peer died */
	unsigned char copy_mode;	/* Copy packet as well as metadata? */
	size_t copy_range;		/* Range past metadata to copy */
	ipq_send_cb_t send;		/* Callback for sending data to peer */
} ipq_peer_t;

typedef struct ipq_queue {
 	int len;			/* Current queue len */
 	int *maxlen;			/* Maximum queue len, via sysctl */
 	unsigned char flushing;		/* If queue is being flushed */
 	unsigned char terminate;	/* If the queue is being terminated */
 	struct list_head list;		/* Head of packet queue */
 	spinlock_t lock;		/* Queue spinlock */
 	ipq_peer_t peer;		/* Userland peer */
    ip_noise_arbitrator_packet_logic_t * packet_logic;
} ipq_queue_t;

static ip_noise_delayer_t * delayer;
/****************************************************************************
 *
 * Packet queue
 *
 ****************************************************************************/
/* Dequeue a packet if matched by cmp, or the next available if cmp is NULL */
static ipq_queue_element_t *
ipq_dequeue(ipq_queue_t *q,
            int (*cmp)(ipq_queue_element_t *, unsigned long),
            unsigned long data)
{
	struct list_head *i;

	spin_lock_bh(&q->lock);
	for (i = q->list.prev; i != &q->list; i = i->prev) {
		ipq_queue_element_t *e = (ipq_queue_element_t *)i;
		
		if (!cmp || cmp(e, data)) {
			list_del(&e->list);
			q->len--;
			spin_unlock_bh(&q->lock);
			return e;
		}
	}
	spin_unlock_bh(&q->lock);
	return NULL;
}

/* Flush all packets */
static void ipq_flush(ipq_queue_t *q)
{
	ipq_queue_element_t *e;
	
	spin_lock_bh(&q->lock);
	q->flushing = 1;
	spin_unlock_bh(&q->lock);
	while ((e = ipq_dequeue(q, NULL, 0))) {
		e->verdict = NF_DROP;
		nf_reinject(e->skb, e->info, e->verdict);
		kfree(e);
	}
	spin_lock_bh(&q->lock);
	q->flushing = 0;
	spin_unlock_bh(&q->lock);
}

static ipq_queue_t *ipq_create_queue(
    nf_queue_outfn_t outfn,
    int *errp, 
    int *sysctl_qmax,
    ip_noise_arbitrator_packet_logic_t * packet_logic
    )
{
	int status;
	ipq_queue_t *q;

	*errp = 0;
	q = kmalloc(sizeof(ipq_queue_t), GFP_KERNEL);
	if (q == NULL) {
		*errp = -ENOMEM;
		return NULL;
	}
	q->len = 0;
	q->maxlen = sysctl_qmax;
	q->flushing = 0;
	q->terminate = 0;
    q->packet_logic = packet_logic;
	INIT_LIST_HEAD(&q->list);
	spin_lock_init(&q->lock);
	status = nf_register_queue_handler(PF_INET, outfn, q);
	if (status < 0) {
		*errp = -EBUSY;
		kfree(q);
		return NULL;
	}
	return q;
}

struct wrapper_struct
{
    ipq_queue_element_t * e;
    struct timer_list * timer;    
};

typedef struct wrapper_struct wrapper_t;

static void release_handler(ip_noise_message_t * e, void * context)
{
    nf_reinject(e->skb, e->info, NF_ACCEPT);
}

/*
 * This function receives a packet. It determines what to do with it
 * by calling ip_noise_arbitrator_packet_logic_decide_what_to_do_with_packet().
 * 
 * Then, if the verdict is accept or drop it does that immidiately. If it
 * is delay, it uses the delayer to delay the function and make sure it is 
 * released when its time comes.
 * */
static int ipq_enqueue(ipq_queue_t *q,
                       struct sk_buff *skb, struct nf_info *info)
{
    ip_noise_verdict_t verdict;
#if 0
	int status;
#endif
	
	spin_lock_bh(&q->lock);
    
    if (q->flushing || q->terminate)
    {
		spin_unlock_bh(&q->lock);
		goto free_drop;
	}

    spin_unlock_bh(&q->lock);

    verdict = 
        ip_noise_arbitrator_packet_logic_decide_what_to_do_with_packet(
                q->packet_logic, 
                skb);

    if (verdict.action == IP_NOISE_VERDICT_ACCEPT)
    {       
        nf_reinject(skb, info, NF_ACCEPT);
    }
    else if (verdict.action == IP_NOISE_VERDICT_DROP)
    {
        nf_reinject(skb, info, NF_DROP);
    }
    else 
    {
        int num_millisecs;
        ip_noise_message_t m;

        /* Determine the delay in msecs */
        num_millisecs = verdict.delay_len;

        if (num_millisecs < 100)
        {
            num_millisecs = 100;
        }

        m.skb = skb;
        m.info = info;

        ip_noise_delayer_delay_packet(delayer, &m, num_millisecs);
    } 
    
    return 0;

free_drop:
	return -EBUSY;
}

static void ipq_destroy_queue(ipq_queue_t *q)
{
	nf_unregister_queue_handler(PF_INET);
	spin_lock_bh(&q->lock);
	q->terminate = 1;
	spin_unlock_bh(&q->lock);
	ipq_flush(q);
	kfree(q);
}

static inline int id_cmp(ipq_queue_element_t *e, unsigned long id)
{
	return (id == (unsigned long )e);
}

static inline int dev_cmp(ipq_queue_element_t *e, unsigned long ifindex)
{
	if (e->info->indev)
		if (e->info->indev->ifindex == ifindex)
			return 1;
	if (e->info->outdev)
		if (e->info->outdev->ifindex == ifindex)
			return 1;
	return 0;
}

/* Drop any queued packets associated with device ifindex */
static void ipq_dev_drop(ipq_queue_t *q, int ifindex)
{
	ipq_queue_element_t *e;
	
	while ((e = ipq_dequeue(q, dev_cmp, ifindex))) {
		e->verdict = NF_DROP;
		nf_reinject(e->skb, e->info, e->verdict);
		kfree(e);
	}
}

/****************************************************************************
 *
 * Netfilter interface
 *
 ****************************************************************************/

/*
 * Packets arrive here from netfilter for queuing to userspace.
 * All of them must be fed back via nf_reinject() or Alexey will kill Rusty.
 */
static int netfilter_receive(struct sk_buff *skb,
                             struct nf_info *info, void *data)
{
	return ipq_enqueue((ipq_queue_t *)data, skb, info);
}

/****************************************************************************
 *
 * Netlink interface.
 *
 ****************************************************************************/

ipq_queue_t *nlq = NULL;

#define RCV_SKB_FAIL(err) do { netlink_ack(skb, nlh, (err)); return; } while (0);

/****************************************************************************
 *
 * System events
 *
 ****************************************************************************/

static int receive_event(struct notifier_block *this,
                         unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;

	/* Drop any packets associated with the downed device */
	if (event == NETDEV_DOWN)
		ipq_dev_drop(nlq, dev->ifindex);
	return NOTIFY_DONE;
}

struct notifier_block ipq_dev_notifier = {
	receive_event,
	NULL,
	0
};

/****************************************************************************
 *
 * Sysctl - queue tuning.
 *
 ****************************************************************************/

static int sysctl_maxlen = IPQ_QMAX_DEFAULT;

static struct ctl_table_header *ipq_sysctl_header;

static ctl_table ipq_table[] = {
	{ NET_IPQ_QMAX, NET_IPQ_QMAX_NAME, &sysctl_maxlen,
	  sizeof(sysctl_maxlen), 0644,  NULL, proc_dointvec },
 	{ 0 }
};

static ctl_table ipq_dir_table[] = {
	{NET_IPV4, "ipv4", NULL, 0, 0555, ipq_table, 0, 0, 0, 0, 0},
	{ 0 }
};

static ctl_table ipq_root_table[] = {
	{CTL_NET, "net", NULL, 0, 0555, ipq_dir_table, 0, 0, 0, 0, 0},
	{ 0 }
};

/****************************************************************************
 *
 * Procfs - debugging info.
 *
 ****************************************************************************/

static int ipq_get_info(char *buffer, char **start, off_t offset, int length)
{
	int len;

	spin_lock_bh(&nlq->lock);
	len = sprintf(buffer,
	              "Queue length        : %d\n"
	              "Queue max. length   : %d\n"
	              "Queue flushing      : %d\n"
	              "Queue terminate     : %d\n",
	              nlq->len,
	              *nlq->maxlen,
	              nlq->flushing,
	              nlq->terminate);
	spin_unlock_bh(&nlq->lock);
	*start = buffer + offset;
	len -= offset;
	if (len > length)
		len = length;
	else if (len < 0)
		len = 0;
	return len;
}

/****************************************************************************
 *
 * Module stuff.
 *
 ****************************************************************************/

extern ip_noise_arbitrator_packet_logic_t * main_init_module(
        ip_noise_arbitrator_iface_t * * iface
        );

/* Some globals which we can later use to deallocate */
static ip_noise_arbitrator_packet_logic_t * packet_logic;
static ip_noise_arbitrator_iface_t * iface;

static int __init init(void)
{
	int status = 0;
	struct proc_dir_entry *proc;
    
    packet_logic = main_init_module(&iface);

    delayer = ip_noise_delayer_alloc(release_handler, NULL);

    printf("ipq_ker_q: delayer=%p\n", delayer);

	nlq = ipq_create_queue(netfilter_receive,
	                       &status, &sysctl_maxlen, packet_logic);
	if (nlq == NULL) {
		printk(KERN_ERR "ip_queue_ker: initialisation failed: unable to "
		       "create queue\n");
		return status;
	}
	proc = proc_net_create(IPQ_PROC_FS_NAME, 0, ipq_get_info);
	if (proc) proc->owner = THIS_MODULE;
	else {
		ipq_destroy_queue(nlq);
		return -ENOMEM;
	}
	register_netdevice_notifier(&ipq_dev_notifier);
	ipq_sysctl_header = register_sysctl_table(ipq_root_table, 0);

	return status;
}

static void __exit fini(void)
{
	unregister_sysctl_table(ipq_sysctl_header);
	proc_net_remove(IPQ_PROC_FS_NAME);
	unregister_netdevice_notifier(&ipq_dev_notifier);
	ipq_destroy_queue(nlq);
    ip_noise_delayer_destroy(delayer);
    ip_noise_arbitrator_data_free(*(packet_logic->data));
    ip_noise_arbitrator_iface_destroy(iface);
}

MODULE_DESCRIPTION("IPv4 in-kernel packet queue handler");
module_init(init);
module_exit(fini);

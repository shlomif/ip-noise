#ifndef __IP_NOISE_K_IPQ
#define __IP_NOISE_K_IPQ

#include <linux/kernel.h>
#include <linux/module.h>

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

#include <linux/netfilter_ipv4/ip_queue.h>
#include <linux/netfilter_ipv4/ip_tables.h>


typedef struct ipq_rt_info {
	__u8 tos;
	__u32 daddr;
	__u32 saddr;
} ipq_rt_info_t;

typedef struct ipq_queue_element {
	struct list_head list;		/* Links element into queue */
	int verdict;			/* Current verdict */
	struct nf_info *info;		/* Extra info from netfilter */
	struct sk_buff *skb;		/* Packet inside */
	ipq_rt_info_t rt_info;		/* May need post-mangle routing */
} ipq_queue_element_t;


#if 0
typedef ipq_queue_element_t ip_noise_ipq_packet_msg_t;
#endif

struct ipq_handle
{
    int hello;
};

#define ipq_perror(str) (printk("IPQ: %s", str))
#define ipq_destroy_handle(handle) 
#define ipq_set_verdict(handle, packet_id, verdict, dc1, dc2) (0)
extern struct ipq_handle * ipq_create_handle(u_int32_t flags);

#define ipq_set_mode(h, mode, range) 0

#endif /* #ifndef __IP_NOISE_K_IPQ */

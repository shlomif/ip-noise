

#ifndef __IP_NOISE_QUEUE_H
#define __IP_NOISE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KERNEL__
#include <linux/netfilter.h>
#include <libipq.h>
#include <sys/time.h>
#else
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>

#include "k_ipq.h"
#include "k_pthread.h"
#endif


 
#define IP_NOISE_MESSAGE_BUFSIZE 0x20000

#ifndef __KERNEL__
struct ip_noise_message_struct
{

    char message[IP_NOISE_MESSAGE_BUFSIZE];

    ip_noise_ipq_packet_msg_t * m;
    struct timeval tv;
    struct ip_noise_message_struct * next;
};
#else
struct ip_noise_message_struct
{
    struct sk_buff * skb;
    struct nf_info * info;
};

#endif

typedef struct ip_noise_message_struct ip_noise_message_t;

#ifndef __KERNEL__

struct ip_noise_messages_queue_struct
{
    ip_noise_message_t * head;
    ip_noise_message_t * tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int num_msgs;
};

typedef struct ip_noise_messages_queue_struct ip_noise_messages_queue_t;

ip_noise_messages_queue_t * ip_noise_messages_queue_alloc(void);

void ip_noise_messages_queue_destroy(ip_noise_messages_queue_t * queue);

ip_noise_message_t * ip_noise_messages_queue_dequeue(ip_noise_messages_queue_t * queue);

void ip_noise_messages_queue_enqueue(ip_noise_messages_queue_t * queue, ip_noise_message_t * msg);

#endif


#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_QUEUE_H */


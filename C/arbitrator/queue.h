

#ifndef __IP_NOISE_QUEUE_H
#define __IP_NOISE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <linux/netfilter.h>
#include <libipq.h>

#define IP_NOISE_MESSAGE_BUFSIZE 0x20000

struct ip_noise_message_struct
{
    char message[IP_NOISE_MESSAGE_BUFSIZE];
    ipq_packet_msg_t * m;
    struct timeval tv;
    struct ip_noise_message_struct * next;
};

typedef struct ip_noise_message_struct ip_noise_message_t;


struct ip_noise_messages_queue_struct
{
    ip_noise_message_t * head;
    ip_noise_message_t * tail;
    pthread_mutex_t mutex;
    int num_msgs;
};

typedef struct ip_noise_messages_queue_struct ip_noise_messages_queue_t;

ip_noise_messages_queue_t * ip_noise_messages_queue_alloc(void);

void ip_noise_messages_queue_destroy(ip_noise_messages_queue_t * queue);

ip_noise_message_t * ip_noise_messages_queue_dequeue(ip_noise_messages_queue_t * queue);

void ip_noise_messages_queue_enqueue(ip_noise_messages_queue_t * queue, ip_noise_message_t * msg);



#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_QUEUE_H */


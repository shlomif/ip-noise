
#ifndef __IP_NOISE_DELAYER_H
#define __IP_NOISE_DELAYER_H

#include <linux/netfilter.h>
#include <libipq.h>
#include <sys/time.h>

#include "pqueue.h"

struct ip_noise_delayer_struct
{
    pthread_mutex_t mutex;
    PQUEUE pq;
};

typedef struct ip_noise_delayer_struct ip_noise_delayer_t;

ip_noise_delayer_t * ip_noise_delayer_alloc(void);

void ip_noise_delayer_delay_packet(
    ip_noise_delayer_t * delayer, 
    ipq_packet_msg_t * m,
    struct timeval tv,
    int delay_len
    );

void ip_noise_delayer_poll(
    ip_noise_delayer_t * delayer
    );
#endif /* #ifndef __IP_NOISE_DELAYER_H */

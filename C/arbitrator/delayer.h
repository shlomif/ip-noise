
#ifndef __IP_NOISE_DELAYER_H
#define __IP_NOISE_DELAYER_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef __KERNEL__
#include <linux/netfilter.h>
#include <libipq.h>
#include <sys/time.h>
#else
#include "k_pthread.h"
#endif

#include "queue.h"
#include "pqueue.h"

struct ip_noise_delayer_struct
{
    pthread_mutex_t mutex;
#ifndef __KERNEL__
    pthread_cond_t cond;
#endif
    PQUEUE pq;
    void (*release_callback)(ip_noise_message_t * m, void * context);
    void * release_callback_context;
    int * terminate;
};

typedef struct ip_noise_delayer_struct ip_noise_delayer_t;

ip_noise_delayer_t * ip_noise_delayer_alloc(
    void (*release_callback)(ip_noise_message_t * m, void * context),
    void * release_callback_context
    );

void ip_noise_delayer_delay_packet(
    ip_noise_delayer_t * delayer, 
    ip_noise_message_t * m,
    struct timeval tv,
    int delay_len
    );

void ip_noise_delayer_loop(
    ip_noise_delayer_t * delayer
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_DELAYER_H */

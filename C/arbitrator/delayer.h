
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
#ifdef __KERNEL__
    struct timer_list current_timer;
    int current_timer_initialized;
#endif

    PQUEUE pq;
    void (*release_callback)(ip_noise_message_t * m, void * context);
    void * release_callback_context;
    int * terminate;
};

typedef struct ip_noise_delayer_struct ip_noise_delayer_t;

extern ip_noise_delayer_t * ip_noise_delayer_alloc(
    void (*release_callback)(ip_noise_message_t * m, void * context),
    void * release_callback_context
    );

extern void ip_noise_delayer_delay_packet(
    ip_noise_delayer_t * delayer, 
    ip_noise_message_t * m,
#ifndef __KERNEL__    
    struct timeval tv,
#endif
    int delay_len
    );

#ifndef __KERNEL__
extern void ip_noise_delayer_loop(
    ip_noise_delayer_t * delayer
    );
#endif

extern void ip_noise_delayer_destroy(ip_noise_delayer_t * delayer);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_DELAYER_H */

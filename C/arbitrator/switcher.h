

#ifndef __IP_NOISE_SWITCHER_H
#define __IP_NOISE_SWITCHER_H

#ifdef __cplusplus
extern "C" {
#endif


#include "iface.h"
#include "rand.h"
#include "pqueue.h"

struct ip_noise_arbitrator_switcher_struct
{
    ip_noise_arbitrator_data_t * * data;
    ip_noise_flags_t * flags;
    int * terminate_ptr;
    PQUEUE pq;
    ip_noise_rand_t * rand;
};

typedef struct ip_noise_arbitrator_switcher_struct ip_noise_arbitrator_switcher_t;

extern ip_noise_arbitrator_switcher_t * ip_noise_arbitrator_switcher_alloc(
    ip_noise_arbitrator_data_t * * data,
    ip_noise_flags_t * flags,
    int * terminate_ptr
    );

extern void ip_noise_arbitrator_switcher_loop(
    ip_noise_arbitrator_switcher_t * self
    );



#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_SWITCHER_H */



#ifndef __IP_NOISE_SWITCHER
#define __IP_NOISE_SWITCHER

#include "iface.h"
#include "rand.h"

struct ip_noise_arbitrator_switcher_struct
{
    ip_noise_arbitrator_date_t * data;
    ip_noise_flags_t * flags;
    int * terminate_ptr;
    PQUEUE pq;
};

typedef struct ip_noise_arbitrator_switcher_struct ip_noise_arbitrator_switcher_t;


#endif /* #ifndef __IP_NOISE_SWITCHER */


#include <stdlib.h>

#include "switcher.h"

struct ip_noise_arbitrator_switcher_event_struct
{
    struct timeval tv;
    int chain;
};

typedef struct ip_noise_arbitrator_switcher_event_struct ip_noise_arbitrator_switcher_event_t;

static int ip_noise_timeval_cmp (void * p_m1, void * p_m2, void * context)
{
    ip_noise_arbitrator_switcher_event_t * m1;
    ip_noise_arbitrator_switcher_event_t * m2;

    m1 = (ip_noise_arbitrator_switcher_event_t * )p_m1;
    m2 = (ip_noise_arbitrator_switcher_event_t * )p_m2;

    if (m1->tv.tv_sec > m2->tv.tv_sec)
    {
        return 1;
    }
    else if (m1->tv.tv_sec < m2->tv.tv_sec)
    {
        return -1;
    }
    else
    {
        if (m1->tv.tv_usec > m2->tv.tv_usec)
        {
            return 1;
        }
        else if (m1->tv.tv_usec < m2->tv.tv_usec)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }    
}

ip_noise_arbitrator_switcher_t * ip_noise_arbitrator_switcher_alloc(
        ip_noise_arbitrator_date_t * data,
        ip_noise_flags_t * flags,
        int * terminate_ptr
        )
{
    ip_noise_arbitrator_switcher_t * self;

    self = malloc(sizeof(ip_noise_arbitrator_switcher_t ));

    self->data = data;
    self->flags = flags;
    self->terminate_ptr = terminate_ptr;

    PQueueInitialize(&(self->pq), 30, 0, ip_noise_timeval_cmp, NULL);
}

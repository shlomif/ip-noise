#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    ip_noise_arbitrator_data_t * data,
    ip_noise_flags_t * flags,
    int * terminate_ptr
    )
{
    ip_noise_arbitrator_switcher_t * self;

    self = malloc(sizeof(ip_noise_arbitrator_switcher_t ));

    self->data = data;
    self->flags = flags;
    self->terminate_ptr = terminate_ptr;
    self->rand = ip_noise_rand_alloc(24);

    PQueueInitialise(&(self->pq), 30, 0, ip_noise_timeval_cmp, NULL);

    return self;
}

void ip_noise_arbitrator_switcher_loop(
    ip_noise_arbitrator_switcher_t * self
    )
{
    ip_noise_flags_t * flags;
    ip_noise_arbitrator_data_t * data;
    ip_noise_rwlock_t * data_lock;
    ip_noise_arbitrator_switcher_event_t current_time_pseudo_msg, * event;
    int chain_index;

    flags = self->flags;
    data = self->data;
    data_lock = data->lock;
    
    while (! *(self->terminate_ptr))
    {
        ip_noise_rwlock_down_read(data_lock);

        if (flags->reinit_switcher)
        {
            printf("Switcher : reinit()!\n");
            reinit(self);
            flags->reinit_switcher = 0;
        }
        gettimeofday(&(current_time_pseudo_msg.tv));

        
        while (!PQueueIsEmpty(&(self->pq)))
        {
            event = PQueuePeekMinimum(&(self->pq));
            if (ip_noise_timeval_cmp (event, &current_time_pseudo_msg) < 0)
            {
                PQueuePop(&(self->pq));
                chain_index = event->chain;
                free(event);
                
                switch_chain(self, chain_index);
                PQueuePush(
                    &(self->pq), 
                    get_new_switch_event(self, chain_index)
                    );
            }
            else
            {
                break;
            }
        }        

        ip_noise_rwlock_up_read(data_lock);

        usleep(50000);
    }
}

#ifndef __KERNEL__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#else
#include "k_stdlib.h"
#include "k_time.h"
#include "k_stdio.h"
#include "k_math.h"
#endif

#include "switcher.h"

struct ip_noise_arbitrator_switcher_event_struct
{
    struct timeval tv;
    int chain;
};

typedef struct ip_noise_arbitrator_switcher_event_struct ip_noise_arbitrator_switcher_event_t;

#ifndef __KERNEL__
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

#endif

ip_noise_arbitrator_switcher_t * ip_noise_arbitrator_switcher_alloc(
    ip_noise_arbitrator_data_t * * data,
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

#ifndef __KERNEL__
    PQueueInitialise(&(self->pq), 30, 0, ip_noise_timeval_cmp, NULL);
#endif

    return self;
}

#ifdef __KERNEL__
struct timer_data_struct
{
    int chain_index;
    ip_noise_arbitrator_switcher_t * self;
};

typedef struct timer_data_struct timer_data_t;

static void switch_chain(
    ip_noise_arbitrator_switcher_t * self, 
    int chain_index
    );

static int calc_chain_delay(
    ip_noise_arbitrator_switcher_t * self,
    int chain_index
    );

static void timer_function(unsigned long ul_data)
{
    timer_data_t * timer_data;
    ip_noise_arbitrator_data_t * data;
    ip_noise_arbitrator_switcher_t * self;
    int chain_index;
    struct timer_list * timer;
    ip_noise_rwlock_t * data_lock;

    timer_data = (timer_data_t*)ul_data;
    self = timer_data->self;
    chain_index = timer_data->chain_index;

    data = *(self->data);
    data_lock = data->lock;

    ip_noise_rwlock_down_read(data_lock);

    switch_chain(self, chain_index);

    timer = &(data->chains[chain_index]->timer);
    init_timer(timer);
    timer->expires = jiffies + (HZ * calc_chain_delay(self, chain_index) / 1000);
    timer->data = (unsigned long)timer_data;
    timer->function = timer_function;
    add_timer(timer);

    ip_noise_rwlock_up_read(data_lock); 
}

#endif



void ip_noise_arbitrator_switcher_reinit(
    ip_noise_arbitrator_switcher_t * self
    )
{
    ip_noise_arbitrator_data_t * data;
    struct timeval tv;
#ifndef __KERNEL__
    struct timezone tz;
#endif
    int chain_index;
#ifndef __KERNEL__ 
    ip_noise_arbitrator_switcher_event_t * event;

    while(! PQueueIsEmpty(&(self->pq)))
    {
        free(PQueuePop(&(self->pq)));
    }
#endif

    data = *(self->data);

    printf("Num chains: %i!\n", data->num_chains);

    gettimeofday(&tv,&tz);

    for(chain_index = 0; chain_index < data->num_chains ; chain_index++)
    {
#ifdef __KERNEL__
        struct timer_list * timer;
        timer_data_t * timer_data;
#endif
        printf("Inputting %i!\n", chain_index);

        data->chains[chain_index]->current_state = 0;
#ifndef __KERNEL__        
        event = malloc(sizeof(ip_noise_arbitrator_switcher_event_t));
        event->tv = tv;
        event->chain = chain_index;

        PQueuePush(&(self->pq), event);
#else
        timer = &(data->chains[chain_index]->timer);
        init_timer(timer);
        timer->expires = jiffies + (HZ * 100 / 1000);
        timer_data = malloc(sizeof(timer_data_t));
        timer_data->self = self;
        timer_data->chain_index = chain_index;
        timer->data = (unsigned long)timer_data;
        timer->function = timer_function;
        add_timer(timer);       
#endif
    }    
}

static void switch_chain(
    ip_noise_arbitrator_switcher_t * self, 
    int chain_index
    )
{
    ip_noise_arbitrator_data_t * data;
    ip_noise_chain_t * chain;
    int current_state;
    ip_noise_move_to_t * move_tos;
    double prob;
    double move_tos_com_prob = 0;
    int i;

    data = *(self->data);
    
    chain = data->chains[chain_index];

    current_state = chain->current_state;

#if 0
    /* At the moment, nothing is implemented using num_move_tos. */
    num_move_tos = chain->states[current_state]->num_move_tos;
#endif
    move_tos = chain->states[current_state]->move_tos;

    prob = ip_noise_rand_rand_in_0_1(self->rand);

    for(i=0;i<chain->num_states;i++)
    {
        move_tos_com_prob += move_tos[i].comulative_prob;
        if (prob < move_tos_com_prob)
        {
            chain->current_state = i;
            printf(
                "Switcher: Switching \"%s\" to \"%s\"\n",
                chain->name,
                chain->states[chain->current_state]->name
                );
            break;
        }
    }
}

#define prob_delta 0.00000000001

static int calc_chain_delay(
    ip_noise_arbitrator_switcher_t * self,
    int chain_index
    )
{
    ip_noise_arbitrator_data_t * data;
    ip_noise_chain_t * chain;
    int current_state;
    int time_factor;
    double prob;
    int length;

    
    data = *(self->data);
    chain = data->chains[chain_index];
    current_state = chain->current_state;

    time_factor = chain->states[current_state]->time_factor;

    prob = ip_noise_rand_rand_in_0_1(self->rand);

    if (prob < prob_delta)
    {
        prob = prob_delta;
    }
    
    length = (int)((-log(prob))*time_factor);

    return length;
}
        
#ifndef __KERNEL__
static ip_noise_arbitrator_switcher_event_t * get_new_switch_event(
    ip_noise_arbitrator_switcher_t * self, 
    int chain_index
    )
{
    int length;
    struct timeval tv;
#ifndef __KERNEL__
    struct timezone tz;
#endif
    ip_noise_arbitrator_switcher_event_t * event;    

    gettimeofday(&tv, &tz);

    length = calc_chain_delay(self, chain_index);

    tv.tv_usec += length*1000;

    if (tv.tv_usec > 1000000)
    {
        tv.tv_sec += tv.tv_usec / 1000000;
        tv.tv_usec %= 1000000;
    }

    event = malloc(sizeof(ip_noise_arbitrator_switcher_event_t));

    event->tv = tv;
    event->chain = chain_index;

    return event;
}
#endif

#ifndef __KERNEL__
static void ip_noise_arbitrator_switcher_poll(
    ip_noise_arbitrator_switcher_t * self
    )
{
    ip_noise_rwlock_t * data_lock;
    ip_noise_arbitrator_data_t * data;
    ip_noise_flags_t * flags;
    ip_noise_arbitrator_switcher_event_t current_time_pseudo_msg, * event;
    struct timezone tz;
    int chain_index;

    data = *(self->data);
    data_lock = data->lock;
    flags = self->flags;
    
    ip_noise_rwlock_down_read(data_lock);

    if (flags->reinit_switcher)
    {
        printf("Switcher : reinit()!\n");
        ip_noise_arbitrator_switcher_reinit(self);
        flags->reinit_switcher = 0;
    }
    gettimeofday(&(current_time_pseudo_msg.tv), &tz);

    
    while (!PQueueIsEmpty(&(self->pq)))
    {
        event = PQueuePeekMinimum(&(self->pq));
        if (ip_noise_timeval_cmp (event, &current_time_pseudo_msg, NULL) < 0)
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
}


void ip_noise_arbitrator_switcher_loop(
    ip_noise_arbitrator_switcher_t * self
    )
{ 
    while (! *(self->terminate_ptr))
    {   
        ip_noise_arbitrator_switcher_poll(self);

        usleep(50000);
    }
}
#endif


#ifndef __KERNEL__
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#else
#include "k_stdlib.h"
#include "k_time.h"
#include "k_stdio.h"
#endif

#include "delayer.h"
#include "pqueue.h"

struct ip_noise_delayer_pq_element_struct
{
    ip_noise_message_t * m;
    struct timeval tv;
};

typedef struct ip_noise_delayer_pq_element_struct ip_noise_delayer_pq_element_t;

static int ip_noise_timeval_cmp (void * p_m1, void * p_m2, void * context)
{
    ip_noise_delayer_pq_element_t * m1;
    ip_noise_delayer_pq_element_t * m2;

    m1 = (ip_noise_delayer_pq_element_t * )p_m1;
    m2 = (ip_noise_delayer_pq_element_t * )p_m2;

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

#ifndef __KERNEL__
extern const pthread_mutex_t ip_noise_global_initial_mutex_constant;
extern const pthread_cond_t ip_noise_global_initial_cond_constant;
#endif

ip_noise_delayer_t * ip_noise_delayer_alloc(
    void (*release_callback)(ip_noise_message_t * m, void * context),
    void * release_callback_context
    )
{
    ip_noise_delayer_t * ret;

    ret = malloc(sizeof(ip_noise_delayer_t));
    printf("ipq_ker_q: ret=%i line=%i module=\"%s\"", 
            (int)ret,
            __LINE__,
            __FILE__
            );

    ret->release_callback = release_callback;
    ret->release_callback_context = release_callback_context;

#ifndef __KERNEL__
    ret->mutex = ip_noise_global_initial_mutex_constant;
#endif
    pthread_mutex_init(&(ret->mutex), NULL);

#ifndef __KERNEL__
    ret->cond = ip_noise_global_initial_cond_constant;
    pthread_cond_init(&(ret->cond), NULL);
#endif

#ifdef __KERNEL__    
    ret->current_timer_initialized = 0;
#endif

    PQueueInitialise(&(ret->pq), 10000, 0, ip_noise_timeval_cmp, NULL );

    return ret;
}

void ip_noise_delayer_destroy(ip_noise_delayer_t * delayer)
{
    pthread_mutex_destroy(&(delayer->mutex));
#ifndef __KERNEL__
    pthread_cond_destroy(&(delayer->cond));
#endif
    PQueueFree(&(delayer->pq));

    free(delayer);
}

void ip_noise_delayer_check_pq(ip_noise_delayer_t * delayer)
{
    ip_noise_delayer_pq_element_t current_time_pseudo_msg, * msg;

    gettimeofday(&(current_time_pseudo_msg.tv), &tz);
    pthread_mutex_lock(&(delayer->mutex));

    while (!PQueueIsEmpty(&(delayer->pq)))
    {
        /* See if the message should have been sent by now. */
        msg = PQueuePeekMinimum(&(delayer->pq));
        if (msg == NULL)
        {
            break;
        }
        if (ip_noise_timeval_cmp(msg, &current_time_pseudo_msg, NULL) < 0)
        {
            delayer->release_callback(msg->m, delayer->release_callback_context);
            PQueuePop(&(delayer->pq));
            free(msg);
        }
        else
        {
            break;
        }       
    }

#ifdef __KERNEL__
    if (!PQueueIsEmpty(&(delayer->pq)))
    {
        unsigned long num_millisecs;

        num_millisecs = 
            (msg->tv.tv_sec - current_time_pseudo_msg.tv.tv_sec) * 1000 + 
            (msg->tv.tv_usec - current_time_pseudo_msg.tv.tv_usec) / 1000000;

        init_timer(&(delayer->current_timer));
        
        delayer->current_timer.expires = jiffies + ((HZ * num_millisecs) / 1000);
        delayer->current_timer.data = (unsigned long)delayer;
        delayer->current_timer.function = (void (*) (unsigned long))ip_noise_delayer_check_pq;
        add_timer(&(delayer->current_timer));
        delayer->current_timer_initialized = 1;
    }
#endif

    pthread_mutex_unlock(&(delayer->mutex));
}

void ip_noise_delayer_delay_packet(
    ip_noise_delayer_t * delayer, 
    ip_noise_message_t * m,
    struct timeval tv,
    int delay_len
    )
{
    ip_noise_delayer_pq_element_t * elem;
    ip_noise_delayer_pq_element_t * min_msg;
    
    tv.tv_usec += delay_len * 1000;
    if (tv.tv_usec > 1000000)
    {
        tv.tv_sec += tv.tv_usec / 1000000;
        tv.tv_usec %= 1000000;
    }

    elem = (ip_noise_delayer_pq_element_t *)malloc(sizeof(ip_noise_delayer_pq_element_t ));

    elem->m = m;
    elem->tv = tv;

    pthread_mutex_lock(&(delayer->mutex));

    min_msg = PQueuePeekMinimum(&(delayer->pq));
    
    PQueuePush(&(delayer->pq), elem);

    if ((min_msg == NULL) ||
        (elem->tv.tv_sec < min_msg->tv.tv_sec) || 
        (
            (elem->tv.tv_sec == min_msg->tv.tv_sec) &&
            (elem->tv.tv_usec < min_msg->tv.tv_usec)
        )
       )
    {
#ifndef __KERNEL__
        pthread_cond_signal(&(delayer->cond));
#else
        if (delayer->current_timer_initialized)
        {
            del_timer(&delayer->current_timer);
        }
        pthread_mutex_unlock(&(delayer->mutex));
        ip_noise_delayer_check_pq(delayer);
        return;        
#endif        
    }
    pthread_mutex_unlock(&(delayer->mutex));
}

#ifndef __KERNEL__
void ip_noise_delayer_loop(
    ip_noise_delayer_t * delayer
    )
{
    struct timezone tz;
    struct timespec ts_to_wait_for;
    int cond_wait_status;

    ip_noise_delayer_pq_element_t current_time_pseudo_msg, * msg;

    while (*(delayer->terminate))
    {
        pthread_mutex_lock(&(delayer->mutex));
        gettimeofday(&(current_time_pseudo_msg.tv), &tz);

        while (!PQueueIsEmpty(&(delayer->pq)))
        {
            /* See if the message should have been sent by now. */
            msg = PQueuePeekMinimum(&(delayer->pq));
            if (ip_noise_timeval_cmp(msg, &current_time_pseudo_msg, NULL) < 0)
            {
                delayer->release_callback(msg->m, delayer->release_callback_context);
                PQueuePop(&(delayer->pq));

                free(msg);
            }
            else
            {
                break;
            }       
        }
        cond_wait_status = 0;
        while (cond_wait_status == 0)
        {
            if (cond_wait_status == 0)
            {
                /* We were signalled from the outside */
                msg = PQueuePeekMinimum(&(delayer->pq));
            }
            if (msg != NULL)
            {
                ts_to_wait_for.tv_sec = msg->tv.tv_sec;
                ts_to_wait_for.tv_nsec = msg->tv.tv_usec * 1000;
            }
            if (PQueueIsEmpty(&(delayer->pq)))
            {
                pthread_cond_wait(
                    &(delayer->cond), 
                    &(delayer->mutex)
                    );
                cond_wait_status = 0;
            }
            else
            {
                cond_wait_status = pthread_cond_timedwait(
                    &(delayer->cond), 
                    &(delayer->mutex),
                    &ts_to_wait_for
                    );
            }            
        } 
        
        pthread_mutex_unlock(&(delayer->mutex));
    }
}
#endif

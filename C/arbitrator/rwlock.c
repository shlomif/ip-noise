/*
 * rwlock.c - Part of the IP-Noise project.
 * Written by Shlomi Fish & Roy Glasberg
 * The Computer Networks Laboratory
 * The Electrical Engineering Department
 * The Technion
 *
 * This code is distributed under the public domain.
 *
 * 
 *
 * Implementation of a Readers-Writers lock for POSIX threads. Plain and 
 * simple. This code is based on the code of the ZThreads library, but it 
 * is not derived from it.
 *
 * */


#ifndef __KERNEL__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#else
#include "k_stdlib.h"
#endif



#include "rwlock.h"
/*
 * Initialize a new readers-writers lock
 * */

#ifndef __KERNEL__
static const pthread_mutex_t mutex_initializer = PTHREAD_MUTEX_INITIALIZER;
static const pthread_cond_t cond_initializer = PTHREAD_COND_INITIALIZER;

ip_noise_rwlock_t * ip_noise_rwlock_alloc()
{
    ip_noise_rwlock_t * lock;

    lock = malloc(sizeof(ip_noise_rwlock_t));

    lock->active_readers = lock->active_writers = 0;
    lock->waiting_readers = lock->waiting_writers = 0;

    lock->mutex_lock = mutex_initializer;
    pthread_mutex_init(&(lock->mutex_lock), NULL);
    
    lock->cond_read = cond_initializer;
    pthread_cond_init(&(lock->cond_read), NULL);

    lock->cond_write = cond_initializer;
    pthread_cond_init(&(lock->cond_write), NULL);

    return lock;
}

void ip_noise_rwlock_free(ip_noise_rwlock_t * lock)
{
    pthread_mutex_destroy(&(lock->mutex_lock));
    pthread_cond_destroy(&(lock->cond_read));
    pthread_cond_destroy(&(lock->cond_write));

    free(lock);
}

inline int ip_noise_rwlock_allow_reader(ip_noise_rwlock_t * lock)
{
    return (lock->active_writers == 0) && (lock->waiting_writers == 0);
}

inline int ip_noise_rwlock_allow_writer(ip_noise_rwlock_t * lock)
{
    return ((lock->active_writers == 0) && (lock->active_readers == 0));
}

void ip_noise_rwlock_down_read(ip_noise_rwlock_t * lock)
{
    int ret = 0;
   
    pthread_mutex_lock(&(lock->mutex_lock));

    lock->waiting_readers++;

    while (! ip_noise_rwlock_allow_reader(lock))
    {

        ret = pthread_cond_wait(&(lock->cond_read), &(lock->mutex_lock));

        if (ret != 0)
        {
            break;
        }
    }

    lock->waiting_readers--;

    if (ret == 0)
    {
        lock->active_readers++;
    }

    pthread_mutex_unlock(&(lock->mutex_lock));
}

void ip_noise_rwlock_down_write(ip_noise_rwlock_t * lock)
{
    int ret = 0;
    
    pthread_mutex_lock(&(lock->mutex_lock));

    lock->waiting_writers++;

    while (! ip_noise_rwlock_allow_writer(lock) )
    {
        ret = pthread_cond_wait(&(lock->cond_write), &(lock->mutex_lock));
        if (ret != 0)
        {
            break;
        }
    }

    lock->waiting_writers--;
    if (ret == 0)
    {
        lock->active_writers++;
    }
    
    pthread_mutex_unlock(&(lock->mutex_lock));
}

void ip_noise_rwlock_up_read(ip_noise_rwlock_t * lock)
{
    pthread_mutex_lock(&(lock->mutex_lock));

    lock->active_readers--;

    lock->fairness_counter--;

    pthread_cond_broadcast(&(lock->cond_write));

    pthread_cond_signal(&(lock->cond_read));

    pthread_mutex_unlock(&(lock->mutex_lock));
}

void ip_noise_rwlock_up_write(ip_noise_rwlock_t * lock)
{
    pthread_mutex_lock(&(lock->mutex_lock));

    lock->active_writers--;
    lock->fairness_counter++;

    /* Notify all */
    pthread_cond_broadcast(&(lock->cond_write));

    pthread_cond_signal(&(lock->cond_read));
    
    pthread_mutex_unlock(&(lock->mutex_lock));
}

#else

/* 
 * This is the code for the kernel. Basically, all it does is initialize
 * a mutex. A mutex can serve as a (very limited) readers-writers lock
 *
 * */

ip_noise_rwlock_t * ip_noise_rwlock_alloc(void)
{
    ip_noise_rwlock_t * ret;

    ret = malloc(sizeof(ip_noise_rwlock_t));
    pthread_mutex_init(ret, NULL);
    
    return ret;
}

void ip_noise_rwlock_free(ip_noise_rwlock_t * lock)
{
    pthread_mutex_destroy(lock);
    free(lock);
}

#endif


/*
 * The rest is a test program to test the rwlock. It can be safely ignored.
 *
 * */
#if 0

ip_noise_rwlock_t * mylock;

#define NUM_READERS 5
#define NUM_WRITERS 2

struct context_struct
{
    int writer;
    int index;
};

typedef struct context_struct context_t;

void * reader_thread(void * void_context)
{
    context_t * context;

    context = (context_t *)void_context;

    while (1)
    {
        ip_noise_rwlock_down_read(mylock);

        printf("Reader %i - Lock!\n", context->index);

        usleep(rand()%1000000);

        ip_noise_rwlock_up_read(mylock);

        printf("Reader %i - Unlock!\n", context->index);

        usleep(rand()%1000000);
    }

    return NULL;
}

void * writer_thread(void * void_context)
{
    context_t * context;

    context = (context_t *)void_context;

    while (1)
    {
        ip_noise_rwlock_down_write(mylock);

        printf("Writer %i - Lock!\n", context->index);

        usleep(rand()%1000000);

        printf("Writer %i - Unlock!\n", context->index);

        ip_noise_rwlock_up_write(mylock);

        usleep(rand()%1000000);
    }

    return NULL;
}


int main(int argc, char * argv[])
{    
    context_t * context;
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int check;

    int a;

    mylock = ip_noise_rwlock_alloc();
    for(a=0;a<NUM_READERS;a++)
    {
        context = malloc(sizeof(context));
        context->index = a;
        context->writer = 0;
        check = pthread_create(
            &readers[a],
            NULL,
            reader_thread,
            context
            );
        
        if (check != 0)
        {
            fprintf(stderr, "Could not create Reader #%i!\n", a);
            exit(-1);
        }
    }

    for(a=0;a<NUM_WRITERS;a++)
    {
        context = malloc(sizeof(context));
        context->index = a;
        context->writer = 0;
        check = pthread_create(
            &writers[a],
            NULL,
            writer_thread,
            context
            );
        
        if (check != 0)
        {
            fprintf(stderr, "Could not create Reader #%i!\n", a);
            exit(-1);
        }
    }

    while(1)
    {
        sleep(1);
    }
    return 0;
}


#endif


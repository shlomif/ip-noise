#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct ip_noise_rwlock_struct
{
    /* Serialize access to this object */
    pthread_mutex_t mutex_lock;
    
    /* Notified on read ready */
    pthread_cond_t cond_read;
    
    /* Notified on write ready */
    pthread_cond_t cond_write;

    /* Track Fairness */
    int fairness_counter;

    /* Track the current readers */

    int active_readers;

    /* Track the current writers */

    int active_writers;

    /* Track the waiting readers */

    int waiting_readers;

    /* Track the waiting writers */

    int waiting_writers;    
};

typedef struct ip_noise_rwlock_struct ip_noise_rwlock_t;

/*
 * Initialize a new readers-writers lock
 * */

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

int ip_noise_rwlock_allow_reader(ip_noise_rwlock_t * lock)
{
    return (lock->active_writers == 0) && (lock->waiting_writers == 0);
}

int ip_noise_rwlock_allow_writer(ip_noise_rwlock_t * lock)
{
    return ((lock->active_writers == 0) && (lock->active_readers == 0));
}

void ip_noise_rwlock_before_read(ip_noise_rwlock_t * lock)
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

void ip_noise_rwlock_before_write(ip_noise_rwlock_t * lock)
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

void ip_noise_rwlock_after_read(ip_noise_rwlock_t * lock)
{
    pthread_mutex_lock(&(lock->mutex_lock));

    lock->active_readers--;

    lock->fairness_counter--;

    pthread_cond_broadcast(&(lock->cond_write));

    pthread_cond_signal(&(lock->cond_read));

    pthread_mutex_unlock(&(lock->mutex_lock));
}

void ip_noise_rwlock_after_write(ip_noise_rwlock_t * lock)
{
    pthread_mutex_lock(&(lock->mutex_lock));

    lock->active_writers--;
    lock->fairness_counter++;

    /* Notify all */
    pthread_cond_broadcast(&(lock->cond_write));

    pthread_cond_signal(&(lock->cond_read));
    
    pthread_mutex_unlock(&(lock->mutex_lock));
}

#if 1

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
        ip_noise_rwlock_before_read(mylock);

        printf("Reader %i - Lock!\n", context->index);

        usleep(rand()%1000000);

        ip_noise_rwlock_after_read(mylock);

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
        ip_noise_rwlock_before_write(mylock);

        printf("Writer %i - Lock!\n", context->index);

        usleep(rand()%1000000);

        printf("Writer %i - Unlock!\n", context->index);

        ip_noise_rwlock_after_write(mylock);

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


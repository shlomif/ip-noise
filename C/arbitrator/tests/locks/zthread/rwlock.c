#include <stdlib.h>
#include <pthread.h>

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
    pthread_cond_destory(&(lock->cond_write));

    free(lock);
}

int ip_noise_rwlock_allow_reader(ip_noise_rwlock_t * lock)
{
    return (lock->active_writers == 0);
}

int ip_noise_rwlock_allow_writer(ip_noise_rwlock_t * lock)
{
    return ((lock->active_writers == 0) && (lock->active_readers == 0));
}

void ip_noise_rw_lock_before_read(ip_noise_rwlock_t * lock)
{
   int ret;
   
   pthread_mutex_lock(&(lock->mutex));

   waiting_readers++;

   ret = pthread_cond_wait(&(lock->cond_read), &(lock->mutex));

   if (ret == 0)
   {
       active_readers++;
   }

   waiting_readers--;

   pthread_mutex_unlock(&(lock->mutex));
}

void ip_noise_rw_lock_before_write(ip_noise_rwlock_t * lock)
{
}

void ip_noise_rw_lock_after_read(ip_noise_rwlock_t * lock)
{
}

void ip_noise_rw_lock_after_write(ip_noise_rwlock_t * lock)
{
}




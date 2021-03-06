
#ifndef __IP_NOISE_RWLOCK_H
#define __IP_NOISE_RWLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KERNEL__
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

extern ip_noise_rwlock_t * ip_noise_rwlock_alloc();
extern void ip_noise_rwlock_free(ip_noise_rwlock_t * lock);

extern void ip_noise_rwlock_down_read ( ip_noise_rwlock_t * lock );
extern void ip_noise_rwlock_down_write ( ip_noise_rwlock_t * lock );
extern void ip_noise_rwlock_up_read ( ip_noise_rwlock_t * lock );
extern void ip_noise_rwlock_up_write ( ip_noise_rwlock_t * lock );
#else

#include "k_pthread.h"

#if 0
typedef pthread_mutex_t ip_noise_rwlock_t;

extern ip_noise_rwlock_t * ip_noise_rwlock_alloc();
extern void ip_noise_rwlock_free(ip_noise_rwlock_t * lock);

#define ip_noise_rwlock_down_read(lock) pthread_mutex_lock(lock)
#define ip_noise_rwlock_down_write(lock) pthread_mutex_lock(lock)
#define ip_noise_rwlock_up_read(lock) pthread_mutex_unlock(lock)
#define ip_noise_rwlock_up_write(lock) pthread_mutex_unlock(lock)
#else

typedef rwlock_t ip_noise_rwlock_t;

extern ip_noise_rwlock_t * ip_noise_rwlock_alloc();
extern void ip_noise_rwlock_free(ip_noise_rwlock_t * lock);

#define ip_noise_rwlock_down_read(lock) read_lock(lock)
#define ip_noise_rwlock_down_write(lock) write_lock(lock)
#define ip_noise_rwlock_up_read(lock) read_unlock(lock)
#define ip_noise_rwlock_up_write(lock) write_unlock(lock)
#endif

#endif




#ifdef __cplusplus
}
#endif

#endif /*#ifndef __IP_NOISE_RWLOCK_H */

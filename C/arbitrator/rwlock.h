
#ifndef __IP_NOISE_RWLOCK_H
#define __IP_NOISE_RWLOCK_H

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

extern void ip_noise_rwlock_before_read(ip_noise_rwlock_t * lock);
extern void ip_noise_rwlock_before_write(ip_noise_rwlock_t * lock);
extern void ip_noise_rwlock_after_read(ip_noise_rwlock_t * lock);
extern void ip_noise_rwlock_after_write(ip_noise_rwlock_t * lock);


#endif /*#ifndef __IP_NOISE_RWLOCK_H */

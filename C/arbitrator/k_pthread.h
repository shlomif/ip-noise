#ifndef __IP_NOISE_K_PTHREAD_H
#define __IP_NOISE_K_PTHREAD_H

#include <asm/semaphore.h>

typedef struct semaphore pthread_mutex_t;

#define pthread_mutex_init(mutex_ptr, null) (init_MUTEX(mutex_ptr))
#define pthread_mutex_lock(mutex_ptr) (up(mutex_ptr))
#define pthread_mutex_unlock(mutex_ptr) (down(mutex_ptr))


#endif /* #ifndef __IP_NOISE_K_PTHREAD_H */

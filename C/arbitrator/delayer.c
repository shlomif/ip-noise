
#include "delayer.h"

ip_noise_delayer_t * ip_noise_delayer_alloc(void)
{
    ip_noise_delayer_t * ret;

    ret = malloc(sizeof(ip_noise_delayer_t));

    ret->mutex = ip_noise_global_initiali_mutex_constant;

    pthread_mutex_init(&(ret->mutex, NULL));



      
}

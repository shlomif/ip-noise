#ifndef __IP_NOISE_RAND_H
#define __IP_NOISE_RAND_H

#ifdef __cplusplus
extern "C" {
#endif


struct ip_noise_rand_struct
{
    long seed;
};

typedef struct ip_noise_rand_struct ip_noise_rand_t;

ip_noise_rand_t * ip_noise_rand_alloc(unsigned int seed);
void ip_noise_rand_free(ip_noise_rand_t * rand);
double ip_noise_rand_rand_in_0_1(ip_noise_rand_t * rand);
extern int ip_noise_rand_rand15(ip_noise_rand_t * rand);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_RAND_H */


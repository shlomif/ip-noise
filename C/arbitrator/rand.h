#ifndef __IP_NOISE_RAND_H
#define __IP_NOISE_RAND_H

struct ip_noise_rand_struct
{
    long seed;
};

typedef struct ip_noise_rand_struct ip_noise_rand_t;

ip_noise_rand_t * ip_noise_rand_alloc(unsigned int seed);
void ip_noise_rand_free(ip_noise_rand_t * rand);
double ip_noise_rand_in_0_1(ip_noise_rand_t * rand);

#endif /* #ifndef __IP_NOISE_RAND_H */

#ifndef __KERNEL__
#include <stdlib.h>
#else
#include "k_stdlib.h"
#endif

#include "rand.h"


ip_noise_rand_t * ip_noise_rand_alloc(unsigned int seed)
{
    ip_noise_rand_t * ret;

    ret = malloc(sizeof(ip_noise_rand_t));
    ret->seed = (long)seed;

    return ret;
}

void ip_noise_rand_free(ip_noise_rand_t * rand)
{
    free(rand);
}

static int ip_noise_rand_rand15(ip_noise_rand_t * rand)
{
    rand->seed = (rand->seed * 214013 + 2531011);
    return (rand->seed >> 16) & 0x7fff;    
}

int ip_noise_rand_rand(ip_noise_rand_t * rand)
{
    int one, two;
    one = ip_noise_rand_rand15(rand);
    two = ip_noise_rand_rand15(rand);

    return (one | (two << 15));
}

void ip_noise_rand_srand(ip_noise_rand_t * rand, int seed)
{
    rand->seed = seed;
}

const int rand_normalizer = 10000000;

double ip_noise_rand_rand_in_0_1(ip_noise_rand_t * rand)
{
    int rand_num;
    double ret;

    rand_num = ip_noise_rand_rand(rand);

    ret = ((double)(rand_num % rand_normalizer)) / rand_normalizer;

    return ret;
}



#include <stdlib.h>

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

void ip_noise_rand_srand(int seed)
{
    rand->seed = seed;
}

#define rand_normalizer 10000000

double ip_noise_rand_in_0_1(ip_noise_rand_t * rand)
{
    int rand;

    rand = ip_noise_rand_rand(rand);

    return (rand % rand_normalizer) * 1.0 / rand_normalizer;
}



/*
 * rand.c - Part of the IP-Noise project.
 * Written by Shlomi Fish & Roy Glasberg
 * The Computer Networks Laboratory
 * The Electrical Engineering Department
 * The Technion
 *
 * (c) 2001
 *
 * This module implements an instance-izable, predictable, pseudo-random
 * number generator.
 *
 * The algorithm used is identical to that of the Microsoft Win32 Run-Time 
 * Library. (don't look at us - it's just that the same algorithm is used
 * to generate the boards of Microsoft Freecell and we had the code for
 * it available as part of Freecell Solver).
 *
 * We cannot use srand() because we wanted several instances of the random
 * number generator to be accessible. Basically, this module should be 
 * replaced by calls to the internal Linux random number generator.
 * 
 * */

#ifndef __KERNEL__
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include "k_stdlib.h"
#include <linux/random.h>
#endif

#include "rand.h"

#ifdef IP_NOISE_DETERMINISTIC_RAND

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

int ip_noise_rand_rand15(ip_noise_rand_t * rand)
{
    rand->seed = (rand->seed * 214013 + 2531011);
    return (rand->seed >> 16) & 0x7fff;    
}

/*
 *
 * This function constructs a larger integral number of out of two
 * 15-bit ones.
 *
 * */
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

#else

ip_noise_rand_t * ip_noise_rand_alloc(unsigned int seed)
{
    ip_noise_rand_t * ret;
    ret = malloc(sizeof(ip_noise_rand_t));

#ifndef __KERNEL__
    ret->fh = open("/dev/urandom", O_RDONLY);
#endif

    return ret;
}

void ip_noise_rand_free(ip_noise_rand_t * rand)
{
#ifndef __KERNEL__
    close(rand->fh);
#endif
    free(rand);
}

int ip_noise_rand_rand(ip_noise_rand_t * rand)
{
    int i;
#ifndef __KERNEL__
    read(rand->fh, &i, 4);
#else
    get_random_bytes(&i, 4);
#endif
        
    i &= 0x3FFFFFFF;

    return i;
}


#endif

const int rand_normalizer = 10000000;

double ip_noise_rand_rand_in_0_1(ip_noise_rand_t * rand)
{
    int rand_num;
    double ret;

    rand_num = ip_noise_rand_rand(rand);

    ret = ((double)(rand_num % rand_normalizer)) / rand_normalizer;

    return ret;
}


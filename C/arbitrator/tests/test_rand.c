#include "rand.h"

int main(int argc, char * argv[])
{
    ip_noise_rand_t * rand;

    rand = ip_noise_rand_alloc(24);
    
    while(1)
    {
        printf("%f\n", ip_noise_rand_rand_in_0_1(rand));
    }

    return 0;
}


#include "k_stdlib.h"

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

void * ip_noise_krealloc(void * orig, size_t orig_size, size_t new_size)
{
    void * new_ptr;
    new_ptr = malloc(new_size);
    memcpy(new_ptr, orig, min(new_size, orig_size));
    free(orig);
    return new_ptr;
}

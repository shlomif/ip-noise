#ifndef __IP_NOISE_K_STDLIB_H
#define __IP_NOISE_K_STDLIB_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#define malloc(num_bytes) (kmalloc(num_bytes, GFP_KERNEL))
#define free(ptr)         (kfree(ptr))

#define ourrealloc(ptr, old_size, new_size) ip_noise_krealloc(ptr, old_size, new_size)

extern void * ip_noise_krealloc(void * ptr, size_t old_size, size_t new_size);

#endif /* #ifndef __IP_NOISE_K_STDLIB_H */

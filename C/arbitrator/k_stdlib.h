#ifndef __IP_NOISE_K_STDLIB_H
#define __IP_NOISE_K_STDLIB_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#define malloc(num_bytes) (kmalloc(num_bytes, GFP_KERNEL))

#endif /* #ifndef __IP_NOISE_K_STDLIB_H */

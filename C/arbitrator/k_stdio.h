#ifndef __IP_NOISE_K_STDIO_H
#define __IP_NOISE_K_STDIO_H

#include <linux/kernel.h>
#include <linux/module.h>

#define printf(fmt, args...) printk(fmt, ## args)

#endif /* #ifndef __IP_NOISE_K_STDIO_H */

#ifndef __IP_NOISE_K_IPQ
#define __IP_NOISE_K_IPQ

#include <linux/kernel.h>
#include <linux/module.h>

struct ipq_packet_msg_struct
{
    int hello;
};

typedef struct ipq_packet_msg_struct ipq_packet_msg_t;

struct ipq_handle
{
    int hello;
};

#define ipq_perror(str) (printk("IPQ: %s", str))

#endif /* #ifndef __IP_NOISE_K_IPQ */

#ifndef __IP_NOISE_K_IPQ
#define __IP_NOISE_K_IPQ

#include <linux/kernel.h>
#include <linux/module.h>

struct ipq_packet_msg_struct
{
    int hello;
    int data_len;
    char * payload;
};

typedef struct ipq_packet_msg_struct ipq_packet_msg_t;

struct ipq_handle
{
    int hello;
};

#define ipq_perror(str) (printk("IPQ: %s", str))
#define ipq_destroy_handle(handle) 
#define ipq_set_verdict(handle, packet_id, verdict, dc1, dc2) (0)
extern struct ipq_handle * ipq_create_handle(u_int32_t flags);

#define ipq_set_mode(h, mode, range) 0

#endif /* #ifndef __IP_NOISE_K_IPQ */

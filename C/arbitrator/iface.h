
#ifndef __IP_NOISE_IFACE_H
#define __IP_NOISE_IFACE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "redblack.h"

#include "rwlock.h"

typedef char ip_noise_id_t[80];

typedef struct rbtree * ip_noise_str2int_dict;

typedef double ip_noise_prob_t;

struct ip_noise_prob_and_delay_struct
{
    ip_noise_prob_t prob;
    int delay;
};

typedef struct ip_noise_prob_and_delay_struct ip_noise_prob_and_delay_t;

struct ip_noise_split_linear_function_struct
{
    int num_points;
    ip_noise_prob_and_delay_t * points;
};

typedef struct ip_noise_split_linear_function_struct ip_noise_split_linear_function_t;

struct ip_noise_delay_struct
{
    int type;
    union params
    {
        double lambda;
        ip_noise_split_linear_function_t split_linear;
    };
};

typedef struct ip_noise_delay_struct ip_noise_delay_t;

struct ip_noise_move_to_struct
{
    ip_noise_prob_t comulative_prob;
    int which_state;
};

typedef struct ip_noise_move_to_struct ip_noise_move_to_t;

struct ip_noise_state_struct
{
    ip_noise_id_t name;
    ip_noise_prob_t drop_prob, delay_prob;
    ip_noise_delay_t delay_function;
    int time_factor;
    int num_move_tos;
    ip_noise_move_to_t * move_tos;
    ip_noise_prob_t stable_delay_prob;
};

typedef struct ip_noise_state_struct ip_noise_state_t;

enum IP_NOISE_WHICH_PACKET_LEN_T
{
    IP_NOISE_WHICH_PACKET_LEN_DONT_CARE,
    IP_NOISE_WHICH_PACKET_LEN_GT, /* Greater than min */
    IP_NOISE_WHICH_PACKET_LEN_LT, /* Lower than max */
    IP_NOISE_WHICH_PACKET_LEN_BETWEEN,
    IP_NOISE_WHICH_PACKET_LEN_NOT_BETWEEN,
};

struct ip_noise_port_range_struct
{
    unsigned short start;
    unsigned short end;
};

typedef struct ip_noise_port_range_struct ip_noise_port_range_t;

struct ip_noise_ip_spec_struct
{
    struct ip_noise_ip_spec * next; /* = NULL to terminate the linked list */

    struct in_addr ip;
    int net_mask; /* The sub-net width of the IP range */
    int num_port_ranges; /* If = 0 then any port will do */
    /* A sorted array of port ranges. We do a binary (or linear) search to see
       if the port of the packet matches them. */
    ip_noise_port_range_t * port_ranges; 
};

typedef struct ip_noise_ip_spec_struct ip_noise_ip_spec_t;

typedef unsigned char ip_noise_protocols_bitmask[32];
typedef unsigned char ip_noise_tos_bit_mask[8];

struct ip_noise_chain_filter_struct
{
    ip_noise_ip_spec_t * source;
    ip_noise_ip_spec_t * dest;
     /* A bit mask specifiying which protocols to filter */
    ip_noise_protocols_bitmask protocols;
    /* A bit mask specifiying which tos to filter */
    ip_noise_tos_bit_mask tos;

    int min_packet_len;
    int max_packet_len;
    int which_packet_len;
};

typedef struct ip_noise_chain_filter_struct ip_noise_chain_filter_t;




struct ip_noise_chain_struct
{
    ip_noise_id_t name;

    int num_states;
    ip_noise_state_t * states;

    int current_state;
    ip_noise_chain_filter_t * filter;

    struct timeval time_of_last_packet;

    ip_noise_str2int_dict state_names;
};

typedef struct ip_noise_chain_struct ip_noise_chain_t;

struct ip_noise_arbitrator_data_struct
{
    int num_chains;
    ip_noise_chain_t * chains;

    ip_noise_rwlock_t lock;

    ip_noise_str2int_dict chain_names;
};

typedef struct ip_noise_arbitrator_struct ip_noise_arbitrator_data_t;

extern ip_noise_str2int_dict ip_noise_str2int_dict_alloc(void);
extern int ip_noise_str2int_dict_get(ip_noise_str2int_dict dict, char * name);
extern void ip_noise_str2int_dict_add(ip_noise_str2int_dict dict, char * name, int index);
extern void ip_noise_str2int_dict_remove(ip_noise_str2int_dict dict, char * name);
extern void ip_noise_str2int_dict_reset(ip_noise_str2int_dict dict);


#endif /* #ifndef __IP_NOISE_IFACE_H */



#include <stdlib.h>

#include "packet_logic.h"

struct ip_noise_packet_info_struct
{
    struct in_addr source_ip, dest_ip;
    int tos;
    int protocol;
    int length;
    int source_port, dest_port;
};

typedef struct ip_noise_packet_info_struct ip_noise_packet_info_t;

extern ip_noise_arbitrator_packet_logic_t * 
    ip_noise_arbitrator_packet_logic_alloc(
        ip_noise_arbitrator_data_t * data,
        ip_noise_flags_t * flags
        )
{
    ip_noise_arbitrator_packet_logic_t * self;
    self = malloc(sizeof(ip_noise_arbitrator_packet_logic_t));

    self->data = data;
    self->flags = flags;

    return self;
}

static ip_noise_packet_info_t * get_packet_info(unsigned char * payload)
{
    ip_noise_packet_info_t * ret;

    ret = malloc(sizeof(ip_noise_packet_info_t ));

    ret->protocol = payload[9];
    memcpy(&(ret->source_ip), &payload[12], sizeof(ret->source_ip));
    memcpy(&(ret->dest_ip), &payload[12], sizeof(ret->dest_ip));
    ret->length = (((int)payload[2])<<8) | payload[3];
    ret->tos = payload[1];

    if (ret->protocol == 6)
    {
        /* This is a TCP packet */
    }
    else if (ret->protocol == 17)
    {
        /* This is a UDP packet */
    }
    else
    {
        ret->source_port = -1;
        ret->dest_port = -1;
    }

    return ret;
}
        
extern ip_noise_verdict_t ip_noise_arbitrator_packet_logic_decide_what_to_do_with_packet(
    ip_noise_arbitrator_packet_logic_t * self,
    ipq_packet_msg_t * msg
    )
{
    ip_noise_verdict_t verdict;
    unsigned char * payload;
    ip_noise_rwlock_t * data_lock;
    ip_noise_packet_info_t * packet_info;

    if (msg->data_len > 0)
    {
        payload = msg->payload;

        data_lock = self->data->lock;

        ip_noise_rwlock_down_read(data_lock);

        packet_info = get_packet_info(payload);

        /* verdict = decide(self, packet_info); */
        verdict.action = IP_NOISE_VERDICT_ACCEPT;
        
        free(packet_info);
        ip_noise_rwlock_up_read(data_lock);
        
        return verdict;        
    }
    else
    {
        /* What should I do with a packet with a data length of 0.
           I know - accept it. */
        verdict.action = IP_NOISE_VERDICT_ACCEPT; 
        return verdict;        
    }
}


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

    if ((ret->protocol == 6) || (ret->protocol == 17))
    {
        /* This is a TCP packet */
        ret->source_port = (((int)payload[20])<<8) | payload[21];
        ret->dest_port = (((int)payload[22])<<8) | payload[23];
    }
    else
    {
        ret->source_port = -1;
        ret->dest_port = -1;
    }

    return ret;
}

static ip_noise_verdict_t chain_decide(
    ip_noise_arbitrator_packet_logic_t * self,
    int chain_index,
    ip_noise_packet_info_t * packet_info,
    int ignore_filter
    )
{
    ip_noise_verdict_t ret;

    ret.action = IP_NOISE_VERDICT_ACCEPT;

    return ret;
}
    

static ip_noise_verdict_t decide(
    ip_noise_arbitrator_packet_logic_t * self,
    ip_noise_packet_info_t * packet_info
    )
{
    ip_noise_arbitrator_data_t * data;
    ip_noise_chain_t * * chains;
    ip_noise_verdict_t global_verdict, chain_verdict;
    int chain_index;

    data = self->data;
    chains = data->chains;

    global_verdict.action = IP_NOISE_VERDICT_ACCEPT;
    global_verdict.delay_len = 0;
    global_verdict.flag = IP_NOISE_VERDICT_FLAG_UNPROCESSED;

    for(chain_index = 1 ; chain_index < data->num_chains ; chain_index++)
    {
        chain_verdict = chain_decide(self, chain_index, packet_info, 0);
        if (chain_verdict.action == IP_NOISE_VERDICT_DROP)
        {
            global_verdict.action = IP_NOISE_VERDICT_DROP;
            global_verdict.flag = IP_NOISE_VERDICT_FLAG_PROCESSED;
        }
        else if (chain_verdict.action == IP_NOISE_VERDICT_DELAY)
        {
            if (global_verdict.action == IP_NOISE_VERDICT_DROP)
            {
                /* Do nothing */
            }
            else if (global_verdict.action == IP_NOISE_VERDICT_DELAY)
            {
                global_verdict.delay_len += chain_verdict.delay_len;
            }
            else if (global_verdict.action == IP_NOISE_VERDICT_ACCEPT)
            {
                global_verdict = chain_verdict;
                global_verdict.flag = IP_NOISE_VERDICT_FLAG_PROCESSED;                
            }
            else
            {
                printf("Unknown global_verdict.action %i!\n", global_verdict.action);
                exit(-1);
            }
        }
        else if (chain_verdict.action == IP_NOISE_VERDICT_ACCEPT)
        {
            if (chain_verdict.flag != IP_NOISE_VERDICT_FLAG_UNPROCESSED)
            {
                global_verdict.flag = IP_NOISE_VERDICT_FLAG_PROCESSED;
            }
        }
        else
        {
            printf("Unknown chain_verdict.action %i!\n", chain_verdict.action);
            exit(-1);
        }
    }

    if (global_verdict.flag == IP_NOISE_VERDICT_FLAG_UNPROCESSED)
    {
        if (data->num_chains > 0)
        {
            global_verdict = chain_decide(self, 0, packet_info, 1);
        }
    }

    if ((global_verdict.action == IP_NOISE_VERDICT_DELAY) &&
        (global_verdict.delay_len == 0))
    {
        global_verdict.action = IP_NOISE_VERDICT_ACCEPT;
    }

    return global_verdict;
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

        printf(
            "SOURCE=%.8X:%i DEST=%.8X:%i PROTO=%i LEN=%i\n", 
            *(int*)&(packet_info->source_ip), packet_info->source_port,
            *(int*)&(packet_info->dest_ip), packet_info->dest_port,
            packet_info->protocol,
            packet_info->length
            );
            

        verdict = decide(self, packet_info);
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


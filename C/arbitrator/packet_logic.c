#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "fcs_dm.h"
#include "rand.h"
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

    self->rand = ip_noise_rand_alloc(5);

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

int compare_prob_and_delay_points(
    const void * v_p1,
    const void * v_p2,
    void * context
    )
{
    ip_noise_prob_and_delay_t * p1 = (ip_noise_prob_and_delay_t * )v_p1;
    ip_noise_prob_and_delay_t * p2 = (ip_noise_prob_and_delay_t * )v_p2;

    if (p1->prob < p2->prob)
    {
        return -1;
    }
    else if (p1->prob > p2->prob)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int is_in_ip_filter(
    ip_noise_ip_spec_t * ip_filter,
    struct in_addr ip_proto,
    int port
    )
{
    unsigned char * ip_bytes;
    unsigned int ip;
    unsigned int spec_ip;
    ip_noise_ip_spec_t * spec;
    int netmask_width;

    ip_bytes = (unsigned char*)&ip_proto;

    ip = (((unsigned int)ip_bytes[0])<<24) |
         (((unsigned int)ip_bytes[1])<<16) |
         (((unsigned int)ip_bytes[2])<<8)  |
         (((unsigned int)ip_bytes[3]))     ; 
        
    if (ip_filter == NULL)
    {
        return 1;
    }
    spec = ip_filter;
    while (spec != NULL)
    {
        netmask_width = spec->net_mask;

        ip_bytes = (unsigned char*)&(spec->ip);
        
        spec_ip = (((unsigned int)ip_bytes[0])<<24) |
                  (((unsigned int)ip_bytes[1])<<16) |
                  (((unsigned int)ip_bytes[2])<<8)  |
                  (((unsigned int)ip_bytes[3]))     ;
        
        if ((spec_ip >> netmask_width) == (ip >> netmask_width))
        {
            if (port == -1)
            {
                return 1;
            }
            else
            {
                int i;
                for( i=0 ; i < spec->num_port_ranges ; i++)
                {
                    if ((spec->port_ranges[i].start <= port) &&
                        (port <= spec->port_ranges[i].end))
                    {
                        return 1;
                    }
                }
            }
        }

        spec = spec->next;
    }
    return 0;
}

static int is_in_chain_filter(
    ip_noise_arbitrator_packet_logic_t * self,
    int chain_index,
    ip_noise_packet_info_t * packet_info
    )
{
    ip_noise_arbitrator_data_t * data;
    ip_noise_chain_t * chain;
    int bit, byte, offset;
    int len_type;
    int p_l;

    data = self->data;

    chain = data->chains[chain_index];

    if (! 
        is_in_ip_filter(
            chain->filter->source,
            packet_info->source_ip,
            packet_info->source_port
            )
        )
    {
        return 0;
    }

    if (! 
        is_in_ip_filter(
            chain->filter->dest,
            packet_info->dest_ip,
            packet_info->dest_port
            )
        )
    {
        return 0;
    }

    offset = packet_info->tos;
    byte = offset >> 3;
    bit = offset & 0x7;
    if (! (chain->filter->tos[byte] & (1<<bit)))
    {
        return 0;
    }

    offset = packet_info->protocol;
    byte = offset >> 3;
    bit = offset & 0x7;
    if (! (chain->filter->protocols[byte] & (1<<bit)))
    {
        return 0;
    }

    len_type = chain->filter->which_packet_len;

    p_l = packet_info->length;

    switch(len_type)
    {
        case IP_NOISE_WHICH_PACKET_LEN_DONT_CARE:
        {
            /* Do Nothing */
        }
        break;

        case IP_NOISE_WHICH_PACKET_LEN_GT:
        {
            if (! (p_l >= chain->filter->min_packet_len))
            {
                return 0;
            }
        }
        break;
        
        case IP_NOISE_WHICH_PACKET_LEN_LT:
        {
            if (! (p_l <= chain->filter->max_packet_len))
            {
                return 0;
            }            
        }
        break;

        case IP_NOISE_WHICH_PACKET_LEN_BETWEEN:
        {
            if (! ((p_l <= chain->filter->max_packet_len) && (p_l >= chain->filter->min_packet_len)))
            {
                return 0;
            }
        }
        break;
        
        case IP_NOISE_WHICH_PACKET_LEN_NOT_BETWEEN:
        {
            if ( ((p_l <= chain->filter->max_packet_len) && (p_l >= chain->filter->min_packet_len)))
            {
                return 0;
            }
            
        }
        break;

        default:
        {
            printf("Uknown len_type == %i!\n", len_type);
            exit(-1);
        }
        break;
    }

    
    return 1;                
}

#define prob_delta 0.00000000001

static ip_noise_verdict_t chain_decide(
    ip_noise_arbitrator_packet_logic_t * self,
    int chain_index,
    ip_noise_packet_info_t * packet_info,
    int ignore_filter
    )
{
    ip_noise_verdict_t ret;
    ip_noise_verdict_t unprocessed_ret;
    ip_noise_chain_t * chain;
    ip_noise_state_t * current_state;
    ip_noise_prob_t which_prob;

    unprocessed_ret.action = IP_NOISE_VERDICT_ACCEPT;
    unprocessed_ret.flag = IP_NOISE_VERDICT_FLAG_UNPROCESSED;

    ret.flag = IP_NOISE_VERDICT_FLAG_PROCESSED;

    if (! ignore_filter)
    {
        if (! is_in_chain_filter(self, chain_index, packet_info))
        {
            return unprocessed_ret;
        }
    }

    chain = self->data->chains[chain_index];
    current_state = chain->states[chain->current_state];

    which_prob = ip_noise_rand_rand_in_0_1(self->rand);

    if (which_prob < current_state->drop_prob)
    {
        ret.action = IP_NOISE_VERDICT_DROP;
        return ret;            
    }
    else if (which_prob < current_state->drop_prob + current_state->delay_prob)
    {
        /* Delay */
        int delay;
        ip_noise_prob_t do_a_stable_delay_prob;
        struct timeval tv, last_tv;
        struct timezone tz;

        if (current_state->delay_function.type == IP_NOISE_DELAY_FUNCTION_EXP)
        {
            ip_noise_prob_t prob;
            int lambda;

            prob = ip_noise_rand_rand_in_0_1(self->rand);

            if (prob < prob_delta)
            {
                prob = prob_delta;                
            }

            lambda = current_state->delay_function.params.lambda;
            delay = (int)((-log(prob)) * lambda);
        }
        else if (current_state->delay_function.type == IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR)
        {
            ip_noise_prob_t prob;
            int num_points;
            ip_noise_prob_and_delay_t * points, pseudo_point, * searched;
            int is_precise;
            
            
            prob = ip_noise_rand_rand_in_0_1(self->rand);

            num_points = current_state->delay_function.params.split_linear.num_points;
            points = current_state->delay_function.params.split_linear.points;

            pseudo_point.prob = prob;
             
            searched = 
                SFO_bsearch(
                    &pseudo_point,
                    points,
                    num_points,
                    sizeof(points[0]),
                    compare_prob_and_delay_points,                    
                    NULL,
                    &is_precise
                    );

            if (is_precise)
            {
                delay = searched->delay;                
            }
            else
            {
                /* This is the formula for linear interpolation. */
                ip_noise_prob_t x1,x2;
                int y1, y2;

                x1 = searched[0].prob;
                y1 = searched[0].delay;
                x2 = searched[1].prob;
                y2 = searched[1].delay;
                
                delay = (int)(((prob-x1)*y1+(x2-prob)*y2)/(x2-x1));
            }
        }
        else
        {
            /* Non-existent delay type - default the delay to 0. */
            delay = 0;
        }

        do_a_stable_delay_prob = ip_noise_rand_rand_in_0_1(self->rand);

        gettimeofday(&tv, &tz);

        if (chain->last_packet_release_time.tv_sec == 0)
        {
            chain->last_packet_release_time = tv;
        }

        if (do_a_stable_delay_prob < current_state->stable_delay_prob)
        {
            /* last_sec and last_usec are the times in which the current
               packet will be released. It is calculated by taking the release
               time of the last packet and adding the delay. */

            last_tv = chain->last_packet_release_time;
            last_tv.tv_usec += delay*1000;
            if (last_tv.tv_usec > 1000000)
            {
                last_tv.tv_sec += last_tv.tv_usec / 1000000;
                last_tv.tv_usec %= 1000000;                
            }

            delay = (last_tv.tv_sec - tv.tv_sec) * 1000 + ((last_tv.tv_usec-tv.tv_usec) / 1000);

            if (delay < 0)
            {
                delay = 0;
            }        
        }

        tv.tv_usec += delay * 1000;
        if (tv.tv_usec > 1000000)
        {
            tv.tv_sec += tv.tv_usec / 1000000;
            tv.tv_usec %= 1000000;
        }
        chain->last_packet_release_time = tv;
        
        ret.action = IP_NOISE_VERDICT_DELAY;
        ret.delay_len = delay;

        return ret;
    }
    else
    {
        ret.action = IP_NOISE_VERDICT_ACCEPT;
        return ret;
    }
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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "conn.h"
#include "iface.h"
#include "rwlock.h"

enum IP_NOISE_RET_VALUE_T
{
    IP_NOISE_RET_VALUE_OK = 0,
    IP_NOISE_RET_VALUE_SOMETHING_WRONG = 1,
    IP_NOISE_RET_VALUE_UNKNOWN_OPCODE = 2,
    IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE = 3,
    IP_NOISE_RET_VALUE_SPLIT_LINEAR_SET_POINTS_ON_OTHER_TYPE = 4,
    IP_NOISE_RET_VALUE_VALUE_OUT_OF_RANGE = 5,
};

ip_noise_arbitrator_data_t * ip_noise_arbitrator_data_alloc(void)
{
    ip_noise_arbitrator_data_t * data;

    data = malloc(sizeof(ip_noise_arbitrator_data_t));

    data->lock = ip_noise_rwlock_alloc();

    data->num_chains = 0;
    data->max_num_chains = 0;
    data->chains = NULL;
    data->chain_names = ip_noise_str2int_dict_alloc();

    return data;
}


static int read_int(
    ip_noise_arbitrator_iface_t * self
    )
{
    unsigned char buffer[4];
    
    ip_noise_conn_read(self->conn, buffer, 4);

    return (       buffer[0] | 
            (((int)buffer[1]) << 8)  | 
            (((int)buffer[2]) << 16) | 
            (((int)buffer[3]) << 24) );
}

static unsigned short read_uint16(
    ip_noise_arbitrator_iface_t * self
    )
{
    unsigned char buffer[2];
    
    ip_noise_conn_read(self->conn, buffer, 2);

    return (                  buffer[0]          | 
            (((unsigned short)buffer[1]) << 8)
           );
}

#define read_opcode(self) (read_int(self))

typedef int param_type_t;

enum IP_NOISE_PARAM_TYPE_T
{
    PARAM_TYPE_STRING,
    PARAM_TYPE_INT,
    PARAM_TYPE_CHAIN,
    PARAM_TYPE_STATE,
    PARAM_TYPE_IP_FILTER,
    PARAM_TYPE_BOOL,
    PARAM_TYPE_WHICH_PACKET_LENGTH,
    PARAM_TYPE_PROB,
    PARAM_TYPE_DELAY_FUNCTION_TYPE,
    PARAM_TYPE_SPLIT_LINEAR_POINTS,
    PARAM_TYPE_LAMBDA,
    PARAM_TYPE_DELAY_TYPE,
    PARAM_TYPE_NONE,
};

union param_union
{
    ip_noise_id_t string;
    int _int;
    int chain;
    int state;
    ip_noise_ip_spec_t * ip_filter;
    int bool;
    int which_packet_length;
    ip_noise_prob_t prob;
    int delay_function_type;
    ip_noise_split_linear_function_t split_linear_points;
    int lambda;
    int delay_type;
};

typedef union param_union param_t;
struct operation_struct
{
    int opcode;
    int num_params;
    param_type_t params[4];
    int num_out_params;
    param_type_t out_params[4];
    int (*handler)(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
};

typedef struct operation_struct operation_t;

static ip_noise_chain_t * chain_alloc(char * name)
{
    ip_noise_chain_t * chain;

    chain = malloc(sizeof(ip_noise_chain_t));

    strncpy(chain->name, name, IP_NOISE_ID_LEN);
    chain->name[IP_NOISE_ID_LEN-1] = '\0';

    chain->states = NULL;
    chain->num_states = chain->max_num_states = 0;
    chain->filter = malloc(sizeof(ip_noise_chain_filter_t));
    chain->filter->source = NULL;
    chain->filter->dest = NULL;
    /* Accept all the protocols */
    memset(chain->filter->protocols, '\xFF', sizeof(chain->filter->protocols));
    /* Accept all the TOS numbers */
    memset(chain->filter->tos, '\xFF', sizeof(chain->filter->tos));
    chain->filter->min_packet_len = 0;
    chain->filter->max_packet_len = 65535;
    chain->filter->which_packet_len = IP_NOISE_WHICH_PACKET_LEN_DONT_CARE;

    chain->state_names = ip_noise_str2int_dict_alloc();

    return chain;
}

static ip_noise_state_t * state_alloc(char * name)
{
    ip_noise_state_t * state;

    state = malloc(sizeof(ip_noise_state_t));

    strncpy(state->name, name, IP_NOISE_ID_LEN);
    state->name[IP_NOISE_ID_LEN-1] = '\0';

    state->drop_prob = 0;
    state->delay_prob = 0;
    state->delay_function.type = IP_NOISE_DELAY_FUNCTION_NONE;
    state->time_factor = 1000;
    state->stable_delay_prob = 0;

    return state; 
}

static void state_free(ip_noise_state_t * state)
{
    if (state->delay_function.type == IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR)
    {
        if(state->delay_function.params.split_linear.points != NULL)
        {
            free(state->delay_function.params.split_linear.points);
        }
    }
    free(state->move_tos);
    free(state);
}

static void ip_spec_free(ip_noise_ip_spec_t * spec)
{
    ip_noise_ip_spec_t * next_spec;

    while(spec != NULL)
    {
        free(spec->port_ranges);
        next_spec = spec->next;
        free(spec);
        spec = next_spec;
    }
}

static void chain_free(ip_noise_chain_t * chain)
{
    int a;

    for(a=0;a<chain->num_states;a++)
    {
        state_free(chain->states[a]);
    }
    
    ip_spec_free(chain->filter->source);
    ip_spec_free(chain->filter->dest);

    free(chain->filter);

    ip_noise_str2int_dict_free(chain->state_names);
    
    free(chain);
}

static ip_noise_chain_t * get_chain(ip_noise_arbitrator_iface_t * self, int chain_index)
{
    ip_noise_arbitrator_data_t * data;

    data = self->data;

    if (chain_index >= data->num_chains)
    {
        return NULL;
    }

    return data->chains[chain_index];
}

static ip_noise_state_t * get_state(ip_noise_arbitrator_iface_t * self, int chain_index, int state_index)
{
    ip_noise_chain_t * chain;

    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return NULL;
    }

    if (state_index >= chain->num_states)
    {
        return NULL;
    }

    return chain->states[state_index];
}

param_t read_param_type(
    ip_noise_arbitrator_iface_t * self,
    param_type_t param_type
    )
{
    ip_noise_conn_t * conn;
    param_t ret;

    conn = self->conn;

    switch(param_type)
    {
        case PARAM_TYPE_STRING:
        {
            ip_noise_conn_read(conn, ret.string, sizeof(ret.string));
            ret.string[IP_NOISE_ID_LEN-1] = '\0';
        }
        break;

        case PARAM_TYPE_INT:
        {
            ret._int = read_int(self);
        }
        break;

        case PARAM_TYPE_CHAIN:
        {
            int which;

            which = read_int(self);

            if (which == 2)
            {
                ret.chain = self->last_chain;
            }
            else
            {
                printf("Uknown chain which %i!\n", which);
                exit(-1);
            }
        }
        break;

        case PARAM_TYPE_STATE:
        {
            int which;

            which = read_int(self);

            if (which == 0)
            {
                int index = read_int(self);

                ret.state = index;
            }
            else if (which == 2)
            {
                ret.state = self->last_state;
            }
            else
            {
                printf("Uknown state which %i!\n", which);
                exit(-1);
            }
            
        }
        break;

        case PARAM_TYPE_PROB:
        {
            double d;
            ip_noise_conn_read(conn, (char *)&d, sizeof(d));
            if ((d < 0) || (d > 1))
            {
                d = 0;
            }
            ret.prob = d;
        }
        break;

        case PARAM_TYPE_DELAY_TYPE:
        {
            int delay;

            delay = read_int(self);
            if (delay <= 0)
            {
                delay = 1000;
            }
            ret.delay_type = delay;            
        }
        break;

        case PARAM_TYPE_DELAY_FUNCTION_TYPE:
        {
            int delay_type = read_int(self);

            if (delay_type == 0)
            {
                ret.delay_function_type = IP_NOISE_DELAY_FUNCTION_EXP;
            }
            else if (delay_type == 1)
            {
                ret.delay_function_type = IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR;
            }
            else 
            {
                ret.delay_function_type = IP_NOISE_DELAY_FUNCTION_NONE;
            }
        }
        break;

        case PARAM_TYPE_IP_FILTER:
        {
            ip_noise_ip_spec_t * head;
            ip_noise_ip_spec_t * tail;
            struct in_addr ip;
            struct in_addr terminator;
            int netmask;
            int num_port_ranges;
            int max_num_port_ranges;
            ip_noise_port_range_t * port_ranges;
            unsigned short start, end;
            
            
            head = malloc(sizeof(ip_noise_ip_spec_t));
            tail = head;
            tail->next = NULL;

            memset(&ip, '\x0', sizeof(ip));
            memset(&terminator, '\xFF', sizeof(terminator));

            while (memcmp(&ip, &terminator, sizeof(ip)) != 0)
            {
                ip_noise_conn_read(conn, (char*)&ip, 4);
                netmask = read_int(self);
                max_num_port_ranges = 16;
                port_ranges = malloc(sizeof(port_ranges[0])*max_num_port_ranges);
                num_port_ranges = 0;
                while (1)
                {
                    start = read_uint16(self);
                    end = read_uint16(self);
                    if (start > end)
                    {
                        break;
                    }
                    if (num_port_ranges == max_num_port_ranges)
                    {
                        max_num_port_ranges += 16;
                        port_ranges = realloc(port_ranges, sizeof(port_ranges[0])*max_num_port_ranges);
                    }                    
                    port_ranges[num_port_ranges].start = start;
                    port_ranges[num_port_ranges].end = end;
                    num_port_ranges++;
                }
                /* Realloc port_ranges to have just enough memory to store all
                 * the port ranges */
                port_ranges = realloc(port_ranges, sizeof(port_ranges[0])*num_port_ranges);
                if (memcmp(&ip, &terminator, sizeof(ip)) != 0)
                {
                    tail->ip = ip;
                    tail->net_mask = netmask;
                    tail->num_port_ranges = num_port_ranges;
                    tail->port_ranges = port_ranges;
                    tail->next = malloc(sizeof(ip_noise_ip_spec_t));
                    tail = tail->next;
                    tail->next = NULL;
                }
            }
            if (num_port_ranges != 0)
            {
                free(port_ranges);
            }
            ret.ip_filter = head;
        }
        break;

        case PARAM_TYPE_SPLIT_LINEAR_POINTS:
        {
            double prob;
            int delay;
            
            int max_num_points;
            ip_noise_split_linear_function_t points;

            max_num_points = 16;
            points.points = malloc(sizeof(points.points[0])*max_num_points);
            points.num_points = 0;

            do 
            {
                prob = read_param_type(self, PARAM_TYPE_PROB).prob;
                delay = read_int(self);

                points.points[points.num_points].prob = prob;
                points.points[points.num_points].delay = delay;

                points.num_points++;
                if (points.num_points == max_num_points)
                {
                    max_num_points += 16;
                    points.points = realloc(points.points, sizeof(points.points[0])*max_num_points);
                }                
            } while ((prob < 1));

            points.points = realloc(points.points, sizeof(points.points[0])*points.num_points);

            ret.split_linear_points = points;
        }
        break;

        case PARAM_TYPE_BOOL:
        {
            ret.bool = (read_int(self) != 0);
        }
        break;

        case PARAM_TYPE_WHICH_PACKET_LENGTH:
        {
            int index = read_int(self);

            if ((index < 0) || (index > 4))
            {
                ret.which_packet_length = IP_NOISE_WHICH_PACKET_LEN_DONT_CARE;
            }
            else
            {
                ret.which_packet_length = index;
            }
        }
        break;

        case PARAM_TYPE_LAMBDA:
        {
            ret.lambda = read_int(self);
        }
        break;
        
        default:
        {
            printf("Uknown Param Type %i!\n", param_type);
            exit(-1);
        }
        break;
    }

    return ret;
}


#include "iface_handlers.c"

static int opcode_compare_wo_context(const void * void_a, const void * void_b)
{
    operation_t * op_a = (operation_t *)void_a;
    operation_t * op_b = (operation_t *)void_b;

    int a = op_a->opcode;
    int b = op_b->opcode;

    if (a < b)
    {
        return -1;
    }
    else if (a > b)
    {
        return 1;
    }
    else
    {
        return 0;
    }    
}

static void write_int(
    ip_noise_arbitrator_iface_t * self,
    int retvalue    
    )
{
    unsigned char buffer[4];
    int a;
    for(a=0;a<4;a++)
    {
        buffer[a] = ((retvalue>>(a*8))&0xFF);
    }
    ip_noise_conn_write(self->conn, buffer, 4);
}


static void write_retvalue(
    ip_noise_arbitrator_iface_t * self,
    int retvalue    
    )
{
    write_int(self, retvalue);
}

static void write_param_type(
    ip_noise_arbitrator_iface_t * self,
    param_type_t param_type,
    param_t value
    )
{
    ip_noise_conn_t * conn;

    conn = self->conn;

    switch(param_type)
    {
        case PARAM_TYPE_INT:
        {
            write_int(self, value._int); 
        }
        break;

        default:
        {
            printf("Unknown out param type: %i!\n", param_type);
            exit(-1);
        }
        break;
    }
}

ip_noise_arbitrator_iface_t * ip_noise_arbitrator_iface_alloc(
    ip_noise_arbitrator_data_t * data,
    ip_noise_flags_t * flags
    )
{
    ip_noise_arbitrator_iface_t * self;

    self = malloc(sizeof(ip_noise_arbitrator_iface_t));

    self->data = data;
    self->flags = flags;
    
    self->last_chain = -1;

    return self;
}
        


void ip_noise_arbitrator_iface_loop(
    ip_noise_arbitrator_iface_t * self
    )
{
    ip_noise_flags_t * flags;
    ip_noise_rwlock_t * data_lock;
    int opcode;
    operation_t * record, opcode_record;
    param_t params[4];
    param_t out_params[4];
    int a;
    int ret_code;

    flags = self->flags;
    data_lock = self->data->lock;

    while (1)
    {
        self->_continue = 1;
        
        printf("%s", "Trying to open a connection!\n");
        self->conn = ip_noise_conn_open();

        /* Gain writer permission to the data */

        printf("%s", "IFace: down_write()!\n");
        ip_noise_rwlock_down_write(data_lock);

        printf("%s", "IFace: gained down_write()!\n");

        while (self->_continue)
        {
            opcode = read_opcode(self);

            opcode_record.opcode = opcode;

            record = bsearch(
                &opcode_record, 
                operations, 
                NUM_OPERATIONS,
                sizeof(operation_t),
                opcode_compare_wo_context
                );

            if (record == NULL)
            {
                printf("Unknown opcode 0x%x!\n", opcode);
                write_retvalue(self, 0x2);
                continue;
            }
            
            /* Read the parameters from the line */
            for(a=0;a<record->num_params;a++)
            {
                params[a] = read_param_type(self, record->params[a]);
            }

            ret_code = record->handler(self, params, out_params);

            write_retvalue(self, ret_code);

            for(a=0;a<record->num_out_params;a++)
            {
                write_param_type(
                    self,
                    record->out_params[a],
                    out_params[a]
                    );
            }
        }
        
        
        flags->reinit_switcher = 1;

        /* Release the data for others to use */
        ip_noise_rwlock_up_write(data_lock);
        
        printf("%s", "IFace: Closing a connection!\n");
        ip_noise_conn_destroy(self->conn);

    }
}

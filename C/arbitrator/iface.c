#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "conn.h"
#include "iface.h"
#include "rwlock.h"

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
        free(state->delay_function.split_linear.points);
    }
    free(state->move_tos);
    free(state);
}

static void chain_free(ip_noise_chain_t * chain)
{
    int a;
    ip_noise_ip_spec_t * spec, * next_spec;

    for(a=0;a<chain->num_states;a++)
    {
        state_free(chain->states[a]);
    }
    
    spec = chain->filter->source;
    while(spec != NULL)
    {
        free(spec->port_ranges);
        next_spec = spec->next;
        free(spec);
        spec = next_spec;
    }

    spec = chain->filter->dest;
    while(spec != NULL)
    {
        free(spec->port_ranges);
        next_spec = spec->next;
        free(spec);
        spec = next_spec;
    }

    free(chain->filter);

    ip_noise_str2int_dict_free(chain->state_names);
    
    free(chain);
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
        
        default:
        {
            printf("Uknown Param Type %i!\n", param_type);
            exit(-1);
        }
    }

    return ret;
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "conn.h"
#include "iface.h"
#include "rwlock.h"

struct ip_noise_flags_struct
{
    int reinit_switcher;    
};

typedef struct ip_noise_flags_struct ip_noise_flags_t;

struct ip_noise_arbitrator_iface_struct
{
    ip_noise_arbitrator_data_t * data;
    int _continue;
    ip_noise_conn_t * conn;
    ip_noise_flags_t * flags;    
};

typedef struct ip_noise_arbitrator_iface_struct ip_noise_arbitrator_iface_t;



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

#include "iface_handlers.c"

void ip_noise_arbitrator_iface_loop(
    ip_noise_arbitrator_iface_t * self
    )
{
    ip_noise_flags_t * flags;
    ip_noise_rwlock_t * data_lock;
    int opcode;

    flags = self->flags;
    data_lock = &(self->data->lock);

    while (1)
    {
        self->_continue = 1;
        
        printf("%s", "Trying to open a connection!\n");
        self->conn = ip_noise_conn_open();

        /* Gain writer permission to the data */

        printf("%s", "IFace: down_write()!\n");
        ip_noise_rwlock_before_write(data_lock);

        printf("%s", "IFace: gained down_write()!\n");

        while (self->_continue)
        {
            opcode = read_opcode(self);
        }
        
        
        flags->reinit_switcher = 1;

        /* Release the data for others to use */
        ip_noise_rwlock_after_write(data_lock);
        
        printf("%s", "IFace: Closing a connection!\n");
        ip_noise_conn_destroy(self->conn);

    }
}

#ifndef __IP_NOISE_PACKET_LOGIC_H

#define __IP_NOISE_PACKET_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "verdict.h"
#include "queue.h"
#include "iface.h"
#include "rand.h"

struct ip_noise_arbitrator_packet_logic_struct
{
    ip_noise_arbitrator_data_t * * data;
    ip_noise_flags_t * flags;
    ip_noise_rand_t * rand;
};

typedef struct ip_noise_arbitrator_packet_logic_struct ip_noise_arbitrator_packet_logic_t;

extern ip_noise_arbitrator_packet_logic_t * 
    ip_noise_arbitrator_packet_logic_alloc(
        ip_noise_arbitrator_data_t * * data,
        ip_noise_flags_t * flags
        );

extern ip_noise_verdict_t ip_noise_arbitrator_packet_logic_decide_what_to_do_with_packet(
    ip_noise_arbitrator_packet_logic_t * self,
    ip_noise_ipq_packet_msg_t * msg
    );



#ifdef __cplusplus
}
#endif

#endif /* #ifndef __IP_NOISE_PACKET_LOGIC_H */



#ifndef __IP_NOISE_VERDICT_H
#define __IP_NOISE_VERDICT_H

enum IP_NOISE_VERDICT_ACTIONS_T
{
    IP_NOISE_VERDICT_ACCEPT,
    IP_NOISE_VERDICT_DROP,
    IP_NOISE_VERDICT_DELAY       
};

enum IP_NOISE_VERDICT_FLAG_T
{
    IP_NOISE_VERDICT_FLAG_UNPROCESSED,
    IP_NOISE_VERDICT_FLAG_PROCESSED
};

struct ip_noise_verdict_struct
{
    int action;
    int delay_len;    
    int flag;
};

typedef struct ip_noise_verdict_struct ip_noise_verdict_t;

#endif /* #ifndef __IP_NOISE_VERDICT_H */

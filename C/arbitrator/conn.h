#ifndef __IP_NOISE_CONN_H
#define __IP_NOISE_CONN_H

struct ip_noise_conn_struct
{
    int in, out;
};

typedef struct ip_noise_conn_struct ip_noise_conn_t;

extern ip_noise_conn_t * ip_noise_conn_open(void);
void ip_noise_conn_destroy(ip_noise_conn_t * conn);
extern int ip_noise_conn_read(ip_noise_conn_t * conn, char * data, int how_much);
extern void ip_noise_conn_write(ip_noise_conn_t * conn, char * data, int len);

    
#endif /* #ifndef __IP_NOISE_CONN_H */


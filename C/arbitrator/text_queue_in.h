#ifndef __IP_NOISE_TEXT_QUEUE_IN_H
#define __IP_NOISE_TEXT_QUEUE_IN_H

#ifdef __cplusplus
extern "C" {
#endif

struct ip_noise_text_queue_in_struct
{
    char * buffer;
    int length;
    int max_size;
    int ptr_offset;
    int is_conn_closed;
};

typedef struct ip_noise_text_queue_in_struct ip_noise_text_queue_in_t;

extern ip_noise_text_queue_in_t * ip_noise_text_queue_in_alloc(void);

extern void ip_noise_text_queue_in_destroy(
    ip_noise_text_queue_in_t * q
    );

extern void ip_noise_text_queue_in_put_bytes(
    ip_noise_text_queue_in_t * q,
    char * bytes,
    int length
    );

extern int ip_noise_text_queue_in_read_bytes(
    ip_noise_text_queue_in_t * q,
    char * dest,
    int num_bytes
    );

extern void ip_noise_text_queue_in_set_conn_closed(
    ip_noise_text_queue_in_t * q
    );

extern void ip_noise_text_queue_in_commit(
    ip_noise_text_queue_in_t * q
    );

extern void ip_noise_text_queue_in_rollback(
    ip_noise_text_queue_in_t * q
    );

#ifdef __cplusplus
};
#endif

#endif /* #ifndef __IP_NOISE_TEXT_QUEUE_IN_H */

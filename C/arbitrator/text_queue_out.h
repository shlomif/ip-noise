#ifndef __IP_NOISE_TEXT_QUEUE_OUT_H
#define __IP_NOISE_TEXT_QUEUE_OUT_H

#ifdef __cplusplus
extern "C" {
#endif

struct ip_noise_text_queue_out_struct
{
    char * buffer;
    int max_size;
    int ptr_offset;
    int length;
};

typedef struct ip_noise_text_queue_out_struct ip_noise_text_queue_out_t;

extern ip_noise_text_queue_out_t * ip_noise_text_queue_out_alloc(void);
extern void ip_noise_text_queue_out_free(ip_noise_text_queue_out_t * q);
extern int ip_noise_text_queue_out_write_bytes(
    ip_noise_text_queue_out_t * q,
    char * dest,
    int num_bytes
    );

extern void ip_noise_text_queue_out_input_bytes(
    ip_noise_text_queue_out_t * q,
    char * src,
    int num_bytes
    );

#ifdef __cplusplus
};
#endif


#endif /* #ifndef __IP_NOISE_TEXT_QUEUE_OUT_H */

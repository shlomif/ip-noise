#include <string.h>
#include <stdlib.h>

#include "text_queue_out.h"

#define IP_NOISE_TEXT_QUEUE_OUT_GROW_BY 2048

ip_noise_text_queue_out_t * ip_noise_text_queue_out_alloc(void)
{
    ip_noise_text_queue_out_t * q;

    q = malloc(sizeof(ip_noise_text_queue_out_t ));
    q->max_size = IP_NOISE_TEXT_QUEUE_OUT_GROW_BY;
    q->buffer = malloc(q->max_size);

    return q;
}

void ip_noise_text_queue_out_free(ip_noise_text_queue_out_t * q)
{
    free (q->buffer);
    free ( q );    
}

int ip_noise_text_queue_out_write_bytes(
    ip_noise_text_queue_out_t * q,
    char * dest,
    int num_bytes
    )
{
    if (q->length - q->ptr_offset < num_bytes)
    {
        num_bytes = q->length - q->ptr_offset;
    }
    memcpy(dest, q->buffer + q->ptr_offset, num_bytes);
    q->ptr_offset += num_bytes;
    
    return num_bytes;
}

void ip_noise_text_queue_out_input_bytes(
    ip_noise_text_queue_out_t * q,
    char * src,
    int num_bytes
    )
{
    if (q->length + num_bytes > q->max_size)
    {
        memmove(q->buffer + q->ptr_offset, q->buffer, q->length - q->ptr_offset);
        q->length -= q->ptr_offset;
        q->ptr_offset = 0;
    }

    if (q->length + num_bytes > q->max_size)
    {
        q->max_size = q->length+num_bytes+IP_NOISE_TEXT_QUEUE_OUT_GROW_BY;
        q->buffer = realloc(q->buffer, q->max_size);        
    }
    memcpy(q->buffer + q->length, src, num_bytes);
    q->length += num_bytes;
}


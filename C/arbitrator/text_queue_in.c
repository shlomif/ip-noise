#include <stdlib.h>
#include <string.h>

#include "read.h"

#define IP_NOISE_TEXT_QUEUE_IN_GROW_BY 2048

struct ip_noise_text_queue_in_struct
{
    char * buffer;
    int length;
    int max_size;
    int ptr_offset;
};

typedef struct ip_noise_text_queue_in_struct ip_noise_text_queue_in_t;

ip_noise_text_queue_in_t * ip_noise_text_queue_in_alloc(void)
{
    ip_noise_text_queue_in_t * q;
    
    q = malloc(sizeof(ip_noise_text_queue_in_t));
    q->max_size = IP_NOISE_TEXT_QUEUE_IN_GROW_BY;
    q->buffer = malloc(q->max_size);
    q->ptr_offset = 0;
    

    return q;
}

void ip_noise_text_queue_in_destroy(
    ip_noise_text_queue_in_t * q
    )
{
    free(q->buffer);
    free(q);
}

void ip_noise_text_queue_in_put_bytes(
    ip_noise_text_queue_in_t * q,
    char * bytes,
    int length
    )
{
    if (q->length + length > q->max_size)
    {
        q->max_size = q->length + length + IP_NOISE_TEXT_QUEUE_IN_GROW_BY;
        q->buffer = realloc(q->buffer, q->max_size);
    }
    memcpy(q->buffer + q->length, bytes, length);
    q->length += length;
}

int ip_noise_text_queue_in_read_bytes(
    ip_noise_text_queue_in_t * q,
    char * dest,
    int num_bytes
    )
{
    /* Check if we don't have enough to accomodate for the user's needs */
    if (q->ptr_offset + num_bytes > q->length)
    {
        return IP_NOISE_READ_NOT_FULLY;
    }
    memcpy(dest, q->buffer + q->ptr_offset, num_bytes);
    q->ptr_offset += num_bytes;

    return IP_NOISE_READ_OK;
}

void ip_noise_text_queue_in_commit(
    ip_noise_text_queue_in_t * q
    )
{
    memmove(q->buffer, q->buffer + q->ptr_offset, q->length - q->ptr_offset);
    q->length -= q->ptr_offset;
    q->ptr_offset = 0;    
}

void ip_noise_text_queue_in_rollback(
    ip_noise_text_queue_in_t * q
    )
{
    q->ptr_offset = 0;
}

#ifndef __KERNEL__
#include <stdlib.h>
#include <string.h>

#include "ourrealloc.h"
#else
#include "k_stdlib.h"
#endif

#include "read.h"
#include "text_queue_in.h"

#define IP_NOISE_TEXT_QUEUE_IN_GROW_BY 2048


ip_noise_text_queue_in_t * ip_noise_text_queue_in_alloc(void)
{
    ip_noise_text_queue_in_t * q;
    
    q = malloc(sizeof(ip_noise_text_queue_in_t));
    q->max_size = IP_NOISE_TEXT_QUEUE_IN_GROW_BY;
    q->buffer = malloc(q->max_size);
    q->ptr_offset = 0;
    q->is_conn_closed = 0;
    q->length = 0;

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
    const char * bytes,
    int length
    )
{
    /*
     * If the end of the data plus the new data exceeds the maximum size - 
     * resize the buffer.
     * */
    if (q->length + length > q->max_size)
    {
        int new_max_size = q->length + length + IP_NOISE_TEXT_QUEUE_IN_GROW_BY;
        q->buffer = ourrealloc(q->buffer, q->max_size, new_max_size);
        q->max_size = new_max_size;
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
        return (q->is_conn_closed) ? IP_NOISE_READ_CONN_TERM : IP_NOISE_READ_NOT_FULLY;
    }
    memcpy(dest, q->buffer + q->ptr_offset, num_bytes);
    q->ptr_offset += num_bytes;

    return IP_NOISE_READ_OK;
}

void ip_noise_text_queue_in_set_conn_closed(
    ip_noise_text_queue_in_t * q
    )
{
    q->is_conn_closed = 1;
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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"

const pthread_mutex_t ip_noise_global_initial_mutex_constant = PTHREAD_MUTEX_INITIALIZER;

ip_noise_messages_queue_t * ip_noise_messages_queue_alloc(void)
{
    ip_noise_messages_queue_t * ret;

    ret = malloc(sizeof(ip_noise_messages_queue_t));

    ret->mutex = ip_noise_global_initial_mutex_constant;

    pthread_mutex_init(&(ret->mutex), NULL);

    ret->head = NULL;
    ret->tail = NULL;

    ret->num_msgs = 0;

    return ret;
}

void ip_noise_messages_queue_destroy(ip_noise_messages_queue_t * queue)
{
    pthread_mutex_destroy(&(queue->mutex));
    free(queue);
}

ip_noise_message_t * ip_noise_messages_queue_dequeue(ip_noise_messages_queue_t * queue)
{
    ip_noise_message_t * ret;
    
    pthread_mutex_lock(&(queue->mutex));

    if (queue->head == NULL)
    {
        ret = NULL;
    }
    else
    {
        ret = queue->head;
        queue->head = ret->next;
        if (queue->head == NULL)
        {
            queue->tail = NULL;
        }

        queue->num_msgs--;
    }
    
    pthread_mutex_unlock(&(queue->mutex));

    return ret;
}

void ip_noise_messages_queue_enqueue(ip_noise_messages_queue_t * queue, ip_noise_message_t * msg)
{
    pthread_mutex_lock(&(queue->mutex));

    if (queue->tail == NULL)
    {
        queue->head = queue->tail = msg;
        msg->next = NULL;
    }
    else
    {
        queue->tail->next = msg;
        queue->tail = msg;
        msg->next = NULL;
    }

    queue->num_msgs++;
    
    pthread_mutex_unlock(&(queue->mutex));
}

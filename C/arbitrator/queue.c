/*
 * queue.c
 * 
 * This module implements a thread-safe queue.
 *
 * */

#ifndef __KERNEL__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"

const pthread_mutex_t ip_noise_global_initial_mutex_constant = PTHREAD_MUTEX_INITIALIZER;

const pthread_cond_t ip_noise_global_initial_cond_constant = PTHREAD_COND_INITIALIZER;

ip_noise_messages_queue_t * ip_noise_messages_queue_alloc(void)
{
    ip_noise_messages_queue_t * ret;

    ret = malloc(sizeof(ip_noise_messages_queue_t));

    ret->mutex = ip_noise_global_initial_mutex_constant;

    pthread_mutex_init(&(ret->mutex), NULL);

    ret->cond = ip_noise_global_initial_cond_constant;

    pthread_cond_init(&(ret->cond), NULL);

    ret->head = NULL;
    ret->tail = NULL;

    ret->num_msgs = 0;

    return ret;
}

void ip_noise_messages_queue_destroy(ip_noise_messages_queue_t * queue)
{
    pthread_mutex_destroy(&(queue->mutex));
    pthread_cond_destroy(&(queue->cond));
    free(queue);
}

ip_noise_message_t * ip_noise_messages_queue_dequeue(ip_noise_messages_queue_t * queue)
{
    ip_noise_message_t * ret;
    
    pthread_mutex_lock(&(queue->mutex));

    /* If there are no messages present in the queue */
    if (queue->head == NULL)
    {
        /* Inifinitely wait for an element to come */
        pthread_cond_wait(&(queue->cond), &(queue->mutex));
    }
    
    /* Retrieve the first element */
    ret = queue->head;
    /* Remove it from the list */
    queue->head = ret->next;
    /* Mark this list is empty if it is indeed so */
    if (queue->head == NULL)
    {
        queue->tail = NULL;
    }

    queue->num_msgs--;
    
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
        /* Append this message to the end of the linked list */
        queue->tail->next = msg;
        /* Mark it as the end */
        queue->tail = msg;
        /* Signify that it is not connected to anything */
        msg->next = NULL;
    }

    queue->num_msgs++;

    if (queue->num_msgs == 1)
    {
        pthread_cond_signal(&(queue->cond));
    }
    
    pthread_mutex_unlock(&(queue->mutex));
}

#endif

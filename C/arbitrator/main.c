#include <linux/netfilter.h>
#include <libipq.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"


static void die(struct ipq_handle * h)
{
    ipq_perror("IP-Noise Arbitrator");
    ipq_destroy_handle(h);
    exit(-1);
}

struct ip_noise_decide_what_to_do_with_packets_thread_context_struct
{
    ip_noise_messages_queue_t * queue;
    int * terminate;
    struct ipq_handle * h;
};

typedef struct ip_noise_decide_what_to_do_with_packets_thread_context_struct ip_noise_decide_what_to_do_with_packets_thread_context_t;

void * ip_noise_decide_what_to_do_with_packets_thread_func (void * void_context)
{
    ip_noise_decide_what_to_do_with_packets_thread_context_t * context;      
    ip_noise_messages_queue_t * packets_to_arbitrate_queue;
    int * terminate;
    ip_noise_message_t * msg_with_time;
    int status;
    struct ipq_handle * h;
    static int num;

    context = (ip_noise_decide_what_to_do_with_packets_thread_context_t * )void_context;

    packets_to_arbitrate_queue = context->queue;
    terminate = context->terminate;
    h = context->h;

    while (! (*terminate))
    {
        msg_with_time = ip_noise_messages_queue_dequeue(packets_to_arbitrate_queue);
        if (msg_with_time == NULL)
        {
            usleep(500);
            continue;
        }
        
        

        printf("Release Packet! (%i)\n", num++);
        status = ipq_set_verdict(h, msg_with_time->m->packet_id, NF_ACCEPT, 0, NULL);

        if (status < 0)
        {
            die(h);
        }

        free(msg_with_time);
    }

    return NULL;
}


int main(int argc, char * argv[])
{
    int status;
    unsigned char message[IP_NOISE_MESSAGE_BUFSIZE];
    struct ipq_handle * h;
    ip_noise_messages_queue_t * packets_to_arbitrate_queue;
    int terminate = 0;
    ip_noise_decide_what_to_do_with_packets_thread_context_t * arbitrator_context;
    pthread_t decide_what_to_with_packets_thread;
    int check;

    h = ipq_create_handle(0);
    if (h == NULL)
    {
        die(h);
    }

    status = ipq_set_mode(h, IPQ_COPY_PACKET, sizeof(message));

    if (status < 0)
    {
        die(h);
    }

    packets_to_arbitrate_queue = ip_noise_messages_queue_alloc();
    
    arbitrator_context = malloc(sizeof(ip_noise_decide_what_to_do_with_packets_thread_context_t));
    arbitrator_context->queue = packets_to_arbitrate_queue ;
    arbitrator_context->h = h;
    arbitrator_context->terminate = &terminate;

    check = pthread_create(
        &decide_what_to_with_packets_thread,
        NULL,
        ip_noise_decide_what_to_do_with_packets_thread_func,
        (void *)arbitrator_context
        );

    if (check != 0)
    {
        fprintf(stderr, "Could not create the arbitrator thread!\n");
        exit(-1);
    }

    do
    {
        status = ipq_read(h, message, sizeof(message), 0);
        if (status < 0)
        {
            die(h);
        }
        switch(ipq_message_type(message))
        {
            case NLMSG_ERROR:
                fprintf(
                    stderr, 
                    "Received error message %d\n",
                    ipq_get_msgerr(message)
                    );
                break;

            case IPQM_PACKET:
            {
                ip_noise_message_t * msg_with_time;
                struct timezone tz;
                static int num = 0;

                msg_with_time = malloc(sizeof(ip_noise_message_t));

                /* We are copying the entire buffer, because otherwise we get
                 * errors, since ipq_get_packet still relies on this buffer
                 * for reference */

                memcpy(msg_with_time->message, message, sizeof(msg_with_time->message));

                msg_with_time->m = ipq_get_packet(msg_with_time->message);
                
                gettimeofday(&(msg_with_time->tv), &tz);
                
                
                ip_noise_messages_queue_enqueue(
                    packets_to_arbitrate_queue,
                    msg_with_time
                    );
                
                printf("Received a message! (%i)\n", num++);

#if 0
                status = ipq_set_verdict(h, m->packet_id, NF_ACCEPT, 0, NULL);

                if (status < 0)
                {
                    die(h);
                }
#endif
                break;
            }

            default:
                fprintf(stderr, "Unknown message type!\n");
                break;                        
        }
    } while (1);

    ipq_destroy_handle(h);

    return 0;
}

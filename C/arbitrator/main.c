#include <linux/netfilter.h>
#include <libipq.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"
#include "delayer.h"
#include "verdict.h"
#include "iface.h"
#include "switcher.h"
#include "packet_logic.h"

#define DEBUG

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
    ip_noise_delayer_t * delayer;
    ip_noise_arbitrator_data_t * data;
    ip_noise_flags_t * flags;
};

typedef struct ip_noise_decide_what_to_do_with_packets_thread_context_struct ip_noise_decide_what_to_do_with_packets_thread_context_t;

static void * ip_noise_decide_what_to_do_with_packets_thread_func (void * void_context)
{
    ip_noise_decide_what_to_do_with_packets_thread_context_t * context;      
    ip_noise_messages_queue_t * packets_to_arbitrate_queue;
    int * terminate;
    ip_noise_message_t * msg_with_time;
    int status;
    struct ipq_handle * h;
#ifdef DEBUG
    static int num;
#endif
    ip_noise_verdict_t verdict;
    ip_noise_delayer_t * delayer;
    ip_noise_arbitrator_data_t * data;
    ip_noise_flags_t * flags;
    ip_noise_arbitrator_packet_logic_t * packet_logic;

    context = (ip_noise_decide_what_to_do_with_packets_thread_context_t * )void_context;

    packets_to_arbitrate_queue = context->queue;
    terminate = context->terminate;
    h = context->h;
    delayer = context->delayer;
    data = context->data;
    flags = context->flags;

    free(context);


    packet_logic = ip_noise_arbitrator_packet_logic_alloc(data, flags);

    while (! (*terminate))
    {
        msg_with_time = ip_noise_messages_queue_dequeue(packets_to_arbitrate_queue);
        if (msg_with_time == NULL)
        {
            usleep(500);
            continue;
        }

#if 0
        verdict = decide_what_to_do_with_packet(msg_with_time->m);
#endif
        verdict = ip_noise_arbitrator_packet_logic_decide_what_to_do_with_packet(packet_logic, msg_with_time->m);
        
        if (verdict.action == IP_NOISE_VERDICT_ACCEPT)
        {
#ifdef DEBUG
#if 0
            printf("Release Packet! (%i)\n", num++);
#endif
#endif
            status = ipq_set_verdict(h, msg_with_time->m->packet_id, NF_ACCEPT, 0, NULL);

            if (status < 0)
            {
                die(h);
            }

            free(msg_with_time);
        }
        else if (verdict.action == IP_NOISE_VERDICT_DROP)
        {
#ifdef DEBUG
            printf("Dropping Packet! (%i)\n", num++);
#endif
            status = ipq_set_verdict(h, msg_with_time->m->packet_id, NF_DROP, 0, NULL);

            if (status < 0)
            {
                die(h);
            }

            free(msg_with_time);
        }
        else if (verdict.action == IP_NOISE_VERDICT_DELAY)
        {
#ifdef DEBUG
            printf("Delaying Packet! (%i)\n", num++);
#endif
            ip_noise_delayer_delay_packet(
                delayer,
                msg_with_time,
                msg_with_time->tv,
                verdict.delay_len
                );
        }
        else
        {
            *terminate = 1;
            fprintf(stderr, "Unknown Action!\n");
            return NULL;
        }
    }

    return NULL;
}

struct ip_noise_release_packets_thread_context_struct
{
    int * terminate;
    ip_noise_delayer_t * delayer;
};

typedef struct ip_noise_release_packets_thread_context_struct ip_noise_release_packets_thread_context_t;

static void * release_packets_thread_func(void * void_context)
{
    ip_noise_release_packets_thread_context_t * context;
    ip_noise_delayer_t * delayer;
    int * terminate;

    context = (ip_noise_release_packets_thread_context_t *)void_context;
    delayer = context->delayer;
    terminate = context->terminate;

    free(context);

    while (! (*terminate) )
    {
        ip_noise_delayer_loop(delayer);
        usleep(500);
    }

    return NULL;
    
}

static void ip_noise_delayer_release_function(ip_noise_message_t * m, void * context)
{
    struct ipq_handle * h = (struct ipq_handle *)context;
    int status;
    status = ipq_set_verdict(h, m->m->packet_id, NF_ACCEPT, 0, NULL);

    if (status < 0)
    {
        die(h);
    }    
    free(m);
}

static void * arb_iface_thread_func(void * context)
{
    ip_noise_arbitrator_iface_loop( (ip_noise_arbitrator_iface_t * )context);

    return NULL;
}

static void * arb_switcher_thread_func(void * context)
{
    ip_noise_arbitrator_switcher_loop( (ip_noise_arbitrator_switcher_t * )context);

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
    
    ip_noise_release_packets_thread_context_t * release_packets_context;
    pthread_t release_packets_thread;
    int check;
    ip_noise_delayer_t * delayer;

    ip_noise_arbitrator_data_t * data;
    ip_noise_flags_t flags;
    ip_noise_arbitrator_iface_t * arb_iface;
    pthread_t arb_iface_thread;

    ip_noise_arbitrator_switcher_t * arb_switcher;
    pthread_t arb_switcher_thread;





    srand(24);

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

    delayer = ip_noise_delayer_alloc(ip_noise_delayer_release_function, (void *)h);
    
    release_packets_context = malloc(sizeof(ip_noise_release_packets_thread_context_t));
    release_packets_context->delayer = delayer;
    release_packets_context->terminate = &terminate;
    
    check = pthread_create(
        &release_packets_thread,
        NULL,
        release_packets_thread_func,
        (void *)release_packets_context
        );

    if (check != 0)
    {
        fprintf(stderr, "Could not create the release packets thread!\n");
        exit(-1);
    }

    data = ip_noise_arbitrator_data_alloc();
    flags.reinit_switcher = 1;

    arb_iface = ip_noise_arbitrator_iface_alloc(data, &flags);

    check = pthread_create(
        &arb_iface_thread,
        NULL,
        arb_iface_thread_func,
        (void *)arb_iface
        );

    if (check != 0)
    {
        fprintf(stderr, "Could not create the arbitrator interface thread!\n");
        exit(-1);
    }

    arb_switcher = ip_noise_arbitrator_switcher_alloc(data, &flags, &terminate);

    check = pthread_create(
        &arb_switcher_thread,
        NULL,
        arb_switcher_thread_func,
        (void *)arb_switcher
        );

    if (check != 0)
    {
        fprintf(stderr, "Could not create the arbitrator switcher thread!\n");
        exit(-1);
    }

    arbitrator_context = malloc(sizeof(ip_noise_decide_what_to_do_with_packets_thread_context_t));
    arbitrator_context->queue = packets_to_arbitrate_queue ;
    arbitrator_context->h = h;
    arbitrator_context->terminate = &terminate;
    arbitrator_context->delayer = delayer;
    arbitrator_context->data = data;
    arbitrator_context->flags = &flags;


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
    

#if 0
    {
        ip_noise_rand_t * rand;

        rand = ip_noise_rand_alloc(24);
    
        while(1)
        {
            printf("%f\n", ip_noise_rand_rand_in_0_1(rand));
            sleep(1);
        }
    }
#endif
    
    do
    {
        status = ipq_read(h, message, sizeof(message), 0);
        if (status < 0)
        {
            /* die(h); */
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
#ifdef DEBUG
#if 0
                static int num = 0;
#endif
#endif

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

#if 0
                printf("Received a message! (%i)\n", num++);
#endif

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

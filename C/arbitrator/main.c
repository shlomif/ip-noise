#include <linux/netfilter.h>
#include <libipq.h>
#include <stdio.h>

#define BUFSIZE 2048

static void die(struct ipq_handle * h)
{
    ipq_perror("IP-Noise Arbitrator");
    ipq_destroy_handle(h);
    exit(-1);
}

int main(int argc, char * argv[])
{
    int status;
    unsigned char message[BUFSIZE];
    struct ipq_handle * h;

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
                ipq_packet_msg_t * m = ipq_get_packet(message);
                
                static int num = 0;
                printf("Received a message! (%i)\n", num++);

                status = ipq_set_verdict(h, m->packet_id, NF_ACCEPT, 0, NULL);

                if (status < 0)
                {
                    die(h);
                }
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

#ifndef __KERNEL__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "conn.h"

#define pipes_dir "/home/project" "/ip-noise/pipes/"

ip_noise_conn_t * ip_noise_conn_open(void)
{
    ip_noise_conn_t * conn;

    conn = malloc(sizeof(ip_noise_conn_t));

    conn->out = open( pipes_dir "/from_arb", O_WRONLY);

    if (conn->out == -1)
    {
        free(conn);
        return NULL;
    }
    conn->in = open( (pipes_dir "/to_arb"), O_RDONLY);

    if (conn->in == -1)
    {
        close(conn->out);
        free(conn);
        return NULL;
    }

    return conn;    
}

void ip_noise_conn_destroy(ip_noise_conn_t * conn)
{
    close(conn->out);
    close(conn->in);

    free(conn);

    /* This is a kludge, but it's here to make sure we do not re-open
       the already open connection immidiately */
    sleep(1);
}

void ip_noise_conn_write(ip_noise_conn_t * conn, char * data, int len)
{
    write(conn->out, data, len);
}

int ip_noise_conn_read(ip_noise_conn_t * conn, char * data, int how_much)
{
    int num_bytes_read;
    num_bytes_read = read(conn->in, data, how_much);
    if (num_bytes_read < how_much)
    {
        return -1;
    }
    else
    {
        return num_bytes_read;
    }
}

#endif

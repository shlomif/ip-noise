#ifndef __KERNEL__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "conn.h"

ip_noise_conn_t * ip_noise_conn_open(void)
{
    ip_noise_conn_t * conn;
    char * local_pipes_dir;
    char * ip_noise_env_var;
    char * pipe_path;

    conn = malloc(sizeof(ip_noise_conn_t));

    ip_noise_env_var = getenv("IP_NOISE_UM_ARB_CONN_PATH");
    if (ip_noise_env_var == NULL)
    {
        local_pipes_dir = "/home/project/ip-noise/pipes/";
    }
    else
    {
        local_pipes_dir = ip_noise_env_var;
    }
    
    pipe_path = malloc(strlen(local_pipes_dir)+100);
    strncpy(pipe_path, local_pipes_dir, strlen(local_pipes_dir)+1);
    strncat(pipe_path, "/from_arb", 99);
    
    conn->out = open( pipe_path, O_WRONLY);
    
    if (conn->out == -1)
    {
        free(pipe_path);
        free(conn);
        return NULL;
    }
    
    strncpy(pipe_path, local_pipes_dir, strlen(local_pipes_dir)+1);
    strncat(pipe_path, "/to_arb", 99);
    
    conn->in = open(pipe_path, O_RDONLY);

    free(pipe_path);

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

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define filename "mylock"

#define NUM_READERS 5
#define NUM_WRITERS 2

struct context_struct
{
    int writer;
    int index;
};

typedef struct context_struct context_t;

void * reader_thread(void * void_context)
{
    context_t * context;
    int fd;

    context = (context_t *)void_context;

    fd = open(filename, O_RDONLY);

    while (1)
    {
        flock(fd, LOCK_SH);

        printf("Reader %i - Lock!\n", context->index);

        usleep(rand()%1000000);

        flock(fd, LOCK_UN);

        printf("Reader %i - Unlock!\n", context->index);

        usleep(rand()%1000000);
    }

    close(fd); 

    return NULL;
}

void * writer_thread(void * void_context)
{
    context_t * context;
    int fd;

    context = (context_t *)void_context;

    fd = open(filename, O_RDONLY);

    while (1)
    {
        flock(fd, LOCK_EX);

        printf("Writer %i - Lock!\n", context->index);

        usleep(rand()%1000000);

        flock(fd, LOCK_UN);

        printf("Writer %i - Unlock!\n", context->index);

        usleep(rand()%1000000);
    }

    close(fd); 

    return NULL;
}

int main(int argc, char * argv[])
{
    context_t * context;
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int check;

    int a;
    for(a=0;a<NUM_READERS;a++)
    {
        context = malloc(sizeof(context));
        context->index = a;
        context->writer = 0;
        check = pthread_create(
            &readers[a],
            NULL,
            reader_thread,
            context
            );
        
        if (check != 0)
        {
            fprintf(stderr, "Could not create Reader #%i!\n", a);
            exit(-1);
        }
    }

    for(a=0;a<NUM_WRITERS;a++)
    {
        context = malloc(sizeof(context));
        context->index = a;
        context->writer = 0;
        check = pthread_create(
            &writers[a],
            NULL,
            writer_thread,
            context
            );
        
        if (check != 0)
        {
            fprintf(stderr, "Could not create Reader #%i!\n", a);
            exit(-1);
        }
    }
    
    while (1)
    {
        sleep(1);
    }
}
  
       

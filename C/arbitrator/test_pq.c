#include <stdio.h>

#include "pqueue.c"

int int_cmp(void * p_i1, void * p_i2, void * context)
{
    int i1 = *(int*)p_i1;
    int i2 = *(int*)p_i2;
    if (i1 < i2)
    {
        return -1;
    }
    else if (i1 > i2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#define NUM_ELEMS 10

int main(int argc, char * argv[])
{
    
    int array[NUM_ELEMS] = { 10,5, 80, 900, 20, 30, 27, 90, 100, 23 };

    int a;

    PQUEUE pq;

    PQueueInitialise(&pq, 10, 0, int_cmp, NULL);

    for(a=0;a<NUM_ELEMS;a++)
    {
        PQueuePush(&pq, &array[a]);
    }
    for(a=0;a<NUM_ELEMS;a++)
    {
        printf("%i\n", *(int*)PQueuePop(&pq));
    }

    return 0;
       
}

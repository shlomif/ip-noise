/*
    pqueue.h - header file for the priority queue implementation.

    Originally written by Justin-Heyes Jones
    Modified by Shlomi Fish, 2000

    This file is in the public domain (it's uncopyrighted).

    Check out Justin-Heyes Jones' A* page from which this code has 
    originated:
        http://www.geocities.com/jheyesjones/astar.html
*/  

#ifndef __PQUEUE_H
#define __PQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "jhjtypes.h"

#include "queue.h"

#ifdef __KERNEL__
typedef unsigned long ip_noise_time_t;
#else
typedef struct timeval ip_noise_time_t;
#endif
    
#ifndef __KERNEL__
typedef void * pq_element_t;
#else

struct ip_noise_delayer_pq_element_struct
{
    ip_noise_message_t m;
    ip_noise_time_t tv;
};

typedef struct ip_noise_delayer_pq_element_struct ip_noise_delayer_pq_element_t;


typedef ip_noise_delayer_pq_element_t pq_element_t; 
#endif

typedef struct _PQUEUE
{
    int32 MaxSize;
    int32 CurrentSize;
    pq_element_t * Elements; /* pointer to void pointers */
    /* pq_rating_t MaxRating; - biggest element possible */
    int IsAscendingHeap; /* true if the heap should be sorted with the maximum scoring elements first */
    int (*cmp)(pq_element_t, pq_element_t, void *); /* A comparison function for comparing
                                          the elements */
    void * context;     /* A context for the comparison function */
} PQUEUE;

/* given an index to any element in a binary tree stored in a linear array with the root at 1 and 
   a "sentinel" value at 0 these macros are useful in making the code clearer */

/* the parent is always given by index/2 */
#define PQ_PARENT_INDEX(i) ((i)>>1)
#define PQ_FIRST_ENTRY (1)

/* left and right children are index * 2 and (index * 2) +1 respectively */
#define PQ_LEFT_CHILD_INDEX(i) ((i)<<1)
#define PQ_RIGHT_CHILD_INDEX(i) (((i)<<)+1)

void PQueueInitialise( 
    PQUEUE *pq, 
    int32 MaxElements, 
    /* pq_rating_t MaxRating, */
    int bIsAscending,
    int (*cmp)(pq_element_t, pq_element_t, void * context),
    void * context
    );

void PQueueFree( PQUEUE *pq );

int PQueuePush( PQUEUE *pq, pq_element_t item/* pq_rating_t*/);

int PQueueIsEmpty( PQUEUE *pq );

pq_element_t PQueuePop( PQUEUE *pq);

pq_element_t PQueuePeekMinimum( PQUEUE * pq);

#define PGetRating(elem) ((elem).rating)


#ifdef __cplusplus
}
#endif

#endif /* #ifdef __PQUEUE_H */

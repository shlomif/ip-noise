#include "k_ipq.h"

struct ipq_handle ipq_create_handle(u_int32_t flags)
{
    struct ipq_handle * ret;

    ret = malloc(sizeof(struct ipq_handle));
    
    ret->hello = 0;

    return ret;
}

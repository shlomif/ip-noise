#include <stdio.h>
#include <math.h>

int main()
{
    double e = exp(1);
    double e_reciprocal = 1/e;

    printf("%.80f\n", e);
    printf("%.80f\n", e_reciprocal);

    return 0;
}

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

extern double mylog(double number);

int main(int argc, char * argv[])
{
    double number;
    double myval, sysval, percent_diff, log_error;
    
    if (argc > 1)
    {
        number = atof(argv[1]);
    }
    else
    {
        number = 516;
    }

    myval = mylog(number);
    sysval = log(number);
    percent_diff = (abs(myval-sysval)/sysval);
    log_error = log10(percent_diff);
    printf("%.80f\n", myval);
    printf("%.80f\n", sysval);
    printf("%i\n", (int)log_error);

    return 0;
}



#include <stdio.h>
#include <stdlib.h>


extern double ___ieee754_log(double value);

int main(int argc, char * argv[])
{
	double value;
	
	value = atof(argv[1]);
	
	printf("%f\n", ___ieee754_log(value));
	
	return 0;
}


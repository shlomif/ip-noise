#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
    int fd;
    double value;
    double ret;

    value = atof(argv[1]);
    fd = open("hello", O_RDWR);
    write(fd, &value, sizeof(value));
    read(fd, &ret, sizeof(ret));
    printf("%f\n", ret);
    close(fd);
        
    return 0;
}



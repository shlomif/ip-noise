#include <stdio.h>

#include "../str2int_dict.c"

int main()
{
    char op[80];
    char name[80];
    int index;
    ip_noise_str2int_dict dict;

    dict = ip_noise_str2int_dict_alloc();

    while (fgets(op, sizeof(op), stdin))
    {
        if (! strncmp(op, "add", 3))
        {
            printf("Please enter the name:\n");
            fflush(stdout);
            fgets(name, sizeof(name), stdin);
            printf("Please enter the index:\n");
            fflush(stdout);
            scanf("%i", &index);
            ip_noise_str2int_dict_add(dict, name, index);            
        }
        else if (!strncmp(op, "get", 3))
        {
            printf("Please enter the name:\n");
            fgets(name, sizeof(name), stdin);
            printf("\n\nThe index is: %i\n", ip_noise_str2int_dict_get(dict, name));                       
        }
        else if (!strncmp(op, "remove", 6))
        {
            printf("Please enter the name:\n");
            fgets(name, sizeof(name), stdin);
            ip_noise_str2int_dict_remove(dict, name);            
        }
        else if (!strncmp(op, "reset", 5))
        {
            ip_noise_str2int_dict_reset(dict);
        }
        else if (!strncmp(op, "exit", 4))
        {
            exit(-1);
        }
    }
}

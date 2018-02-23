#include <stdio.h>

static int pr_ena;
    
#define prdbg(...) do { \
    if(pr_ena) \
        printf(__VA_ARGS__); \
    } while (0)

struct dbgtmo {
    int count;
    char *msg;
    int msg_printed;
};
    

static struct dbgtmo mytmo = {
    .count = 0,
    .msg = "My message\n"
};

int main (int argc, char **argv)
{
    char str[80];
    
    sprintf(str, "foobar\n");
    
    pr_ena = 1;  
    
    prdbg("There are %d arguments\n", argc);
    prdbg("Buffer is %d bytes\n", sizeof(str));
    
    return 0;
}

#include <stdio.h>

struct foo {
	unsigned int f1 : 1;
	unsigned int f2 : 1;
	unsigned int f3 : 7;
} foo1;

main ()
{
	int size = sizeof(foo1);
	printf("\n%d \n\n", size);
}


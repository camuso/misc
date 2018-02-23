
#include <stdio.h>
#include <string.h>

static struct yada {
	int size;
	void *foo;
	void *bar;
} yada = {.size = sizeof(struct yada)};

int main ()
{
	printf("sizeof struct yada: %d\n", yada.size);
	return 0;
}

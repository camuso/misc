#include <stdio.h>

struct foo {
	int foonum;
	char *foostr;
};

struct foo fooarray[] = {
	{1, "foo1"},
	{2, "foo2"},
	{3, "foo3"},
};

struct bar {
	char *barstr;
};

struct bar bararray[] = {
	[1] = "bar1",
	[2] = "bar3",
	[4] = "bar4",
	[8] = "bar8",
};

int main(int argc, char** argv)
{
	struct foo *fooptr = fooarray;
	int foosize = sizeof(struct foo);
	int fooarraysize = sizeof(fooarray);
	int fooarrayelements = sizeof(fooarray) / sizeof(struct foo);
	int i;

	printf("\nsize of foo: %d  size of fooarray: %d  number of elements: %d \n\n",
		foosize, fooarraysize, (fooarraysize / foosize));

	for (i = 0; i < fooarrayelements; i++)
		printf("%s: %d\n", fooarray[i].foostr, fooarray[i].foonum);

	putchar('\n');

	for (i = 1<<0; i <= 8; i <<= 1)
		printf("%s: %d\n", bararray[i].barstr, i);

	return 0;
}

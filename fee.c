#include <stdio.h>

#define max(a, b)((a) > (b) ? (a) : (b))
#define Name Manish

struct fields {
	unsigned char f1 : 1;
	unsigned char f2 : 1;
	unsigned short f3 : 7;
} fields1;

main ()
{
	struct node {
		int count;
		char *word;
		struct node *next;
	}Node;

	int size = sizeof(fields1);
	

	int array[] = {1, 2, 3, 4};
	int a=4, c=5;

	int i = 4, *j, *k;
	int n=5,x; x = (n+x); x++;


	j = &i;
	j = j+4;
	j = j - 2;
	k = j + 3;
	//j = j * 2;

	printf("\n%d \n\n", size);

	printf("%d\n", array[3]);
	//printf("%d\n", array[5]);

	printf("My name"       " Name");

	printf("%d ", max(a++, c++));
	printf("%d %d\n", a, c);
}


#include <stdio.h>

main()
{
	struct mystruct {int a; int b; int c; int d; int e;};
	struct mystruct *mystruct_p;

	printf("  sizeof(unsigned char) =  %d\n", sizeof(unsigned char));
	printf("sizeof(unsigned char *) =  %d\n\n", sizeof(unsigned char *));

	printf("sizeof(struct mystruct) = %d\n", sizeof(struct mystruct));
	printf("    sizeof(*mystruct_p) = %d\n", sizeof(*mystruct_p));
	printf("     sizeof(mystruct_p) = %d\n", sizeof(mystruct_p));
}


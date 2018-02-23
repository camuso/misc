#include <stdio.h>

int myfunc (int myint);


main()
{
	printf("myint=%d\n", myfunc(3));
}

int myfunc (int myint)
{
	return myint+1;
}


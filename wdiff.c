// wdiff.c
//
// Tony Camuso
// 2012-04-29
//
// Compare the white spaces in two files, line by line. 
// The two files are expected to be versions of the same file. 
// The idea is to identify places where tabs have been substituted for 
// spaces and vice versa.
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define errexit(str, enr) exit(usage(str, enr))
#define BUFSIZ 256

enum err_t { ENARGS=1, EFILE, EUSER};

static char *errmsg[]={
	"",
	"\tIncorrect number of arguments: ",
	"\tCannot open file to read as text: ",
	"\tUser exited.",
};


static int usage(const char *str, const int enr)
{
	printf("usage: wdiff fileA fileB\n"
	       "\tDiff white space in two closely related text files.\n"
	       "%s%s\n", errmsg[enr], str);
	return enr;
}


static inline int getfilesize(FILE* file)
{
	int len;
	fseek(file, 0L, SEEK_END);
	len = ftell(file);
	rewind(file);
	return len;
}

static int comparefilesizes(FILE* file_a, FILE* file_b)
{
	int   sizeof_file_a = getfilesize(file_a);
	int   sizeof_file_b = getfilesize(file_b);
	return sizeof_file_a - sizeof_file_b;
}

static int compareline(FILE* file_a, FILE* file_b)
{
	char buff_a[BUFSIZ];
	char buff_b[BUFSIZ];
	
	fgets(buff_a, BUFSIZ, file_a);
	fgets(buff_b, BUFSIZ, file_b);
	
	
	return 0;
}

int main(int argc, char *argv[])
{
	FILE* file_a;
	FILE* file_b;
	int sizediff;
	char szArgs[8];
	
	// Express the number of arguments as a string.
	//
	sprintf(szArgs, "%d", argc);
	
	// Only accept two filenames as arguments. 
	//
	if (argc != 3)
		errexit(szArgs, ENARGS);
		
	if ((file_a = fopen(argv[1], "r")) == NULL)
		errexit(argv[1], EFILE);
		
	if ((file_b = fopen(argv[2], "r")) == NULL)
		errexit(argv[2], EFILE);

	if (sizediff = comparefilesizes(file_a, file_b))
		printf("File sizes differ by %d bytes.\n", sizediff);
		
	
		
	return 0;
}
	
		
	
	


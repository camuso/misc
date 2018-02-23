static void map_file(const char *filename)
{
	FILE *file = fopen(filename, "r");
	int fd = fileno(file);
	size_t fsize;
	struct stat st;
	char *str;

	stat(filename, &st);
	fsize = st.st_size;
	fmap = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
	str = *((char**)fmap);
	puts(str);
}

bool open_file(FILE **f, char *filename)
{
	if (!(*f = fopen(filename, "a+"))) {
		printf("Cannot open file \"%s\".\n", filename);
		return false;
	}
	return true;
}


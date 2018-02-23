static char *safe_strcat(char *dest, const char *src, size_t bufsiz)
{
	int space = bufsiz - strlen(dest) - strlen(src) - 1;
	if (space < 1)
		return NULL;
	return strncat(dest, src, space);
}


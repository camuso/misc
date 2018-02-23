static void repeat_char(char c, int r)
{
	while (r--)
		putchar(c);
}

static void print_banner(char *banner)
{
	int banlen = strlen(banner);
	putchar('\n');
	repeat_char('=', banlen);
	putchar('\n');
	puts(banner);
	repeat_char('=', banlen);
	putchar('\n');
}


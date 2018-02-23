static long convert_decimal_text (char *text)
{
	int len = strlen(text);
	long temp = 0;
	int decplace = 1;

	for (i = 0; i < len; ++i) {
		byte = text & 0xFF;
		temp += strtol(byte, 0, 0) * decplace;
		decplace *= 10;
		text >>= 8;
	}


}


/*****************************************************
** Output utilities
******************************************************/

#if !defined(NDEBUG)	// see /usr/include/assert.h
static int count_bits(unsigned long mask)
{
	int count = 0;

	do {
		count += mask & 1;
	} while (mask >>= 1);

	return count;
}
#endif

char *get_prefix(enum ctlflags flags)
{
	// These flags are mutually exclusive.
	//
	unsigned long flg = flags &
			(CTL_FILE 	|
			 CTL_EXPORTED 	|
			 CTL_RETURN	|
			 CTL_ARG 	|
			 CTL_NESTED);

	assert(count_bits(flg) <= 1);

	switch (flg) {
	case CTL_FILE:
		return "FILE";
	case CTL_EXPORTED:
		return "EXPORTED";
	case CTL_RETURN:
		return "RETURN";
	case CTL_ARG:
		return "ARG";
	case CTL_NESTED:
		return "NESTED";
	}
	return "";
}


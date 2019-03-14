//
// pio_ping.c - read a Port IO address supplied at the command line.
//
// Example:
// 	pio ca2
//
// 	Reads Port IO address given as an argument
//
// 	All inputs must be a valid hex number ranging from 0 to 0xFFF
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <asm/errno.h>	// includes /usr/include/asm-generic/errno.h and
			// /usr/include/asm-generic/errno-base.h

#define PIO_MAXOFFSET 0xFFF

int main(int argc, char *argv[])
{
	char 		*pio_addrstr = argv[1];
	char 		*pio_filename = "/dev/port";
	int		pio_filedesc;
	unsigned char 	pio_byte;
	long		pio_offset;
	int		status;

	if (argc != 2) {
		printf("Wrong number of argments: %d\n", argc);
		exit(EINVAL);
	}

	if (pio_addrstr[strspn(pio_addrstr, "0123456789abcdefABCDEF")] != '\0') {
		printf("%s is not a valid hexadecimal number.\n", pio_addrstr);
		exit(EINVAL);
	}

	pio_offset = strtol(pio_addrstr, NULL, 16);
	if (pio_offset > 0xFFF) {
		printf("Port IO offset: %x greater than allowed: %x\n",
			pio_offset, PIO_MAXOFFSET);
		exit(EFAULT);
	}

	pio_filedesc = open(pio_filename, O_RDONLY);
	if (pio_filedesc < 0) {
		printf("Cannot open %s\n", pio_filename);
		exit(ENOENT);
	}

	printf("Reading from Port IO addr %x\n", pio_offset);
	lseek(pio_filedesc, pio_offset, SEEK_SET);
	status = read(pio_filedesc, &pio_byte, 1, 1);

	if (status <= 0) {
		printf("Read from Port IO addr %x failed.\n", pio_offset);
		close(pio_filedesc);
		exit(ESPIPE);
	}

	printf("Byte read from Port IO addr: %x is %x\n", pio_offset, pio_byte);
	close(pio_filedesc);

	return 0;
}

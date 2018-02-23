/* compile with: gcc -Wall -O2 -o tsc-dump tsc-dump.c
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned long long cycles_t;

/* read the tsc */
#define rdtscll()					\
({							\
	cycles_t __t;					\
	__asm__ __volatile__("rdtsc" : "=A" (__t));	\
	__t;						\
})

int main(int argc, char **argv)
{
	int i;
	cycles_t t0, t0_prev = 0, delta = 0;

	while (1) {
		t0 = rdtscll();
		if (t0_prev)
			delta = t0 - t0_prev;
		t0_prev = t0;

		printf("%20Ld / %016Lx [delta: %Ld]\n", t0, t0, delta);
		if (delta < 0) {
			/* this is really really bad */
			return 0;
		}
	}

	return 0;
}


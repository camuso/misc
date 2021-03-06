/* compile with: gcc -Wall -O2 -o tsc-dump tsc-dump.c
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
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
	// TSC stuff
	//
	int i;
	cycles_t t0, t0_prev = 0, delta = 0;

	// gettimeofday() stuf
	//
	struct timeval tv ;
	double delayMicroSec;
	long i=0;


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

#if 0
int main(int argc, char* argv[])
{
   struct timeval tv ;
   double delayMicroSec;
   long i=0;

   /* grab start time */
   gettimeofday( &tv, NULL ) ;
   long start=(( tv.tv_sec ) * 1000000000 ) + ( tv.tv_usec * 1000 ) ;

  /* run through loop */

   for (i=0; i<100000000; i++) {
       // This loopsize should go less than a minute and more than a couple seconds.
       gettimeofday( &tv, NULL );
       }

   /* grab end time-- probably don't need an extra lookup, but whatever */
   gettimeofday( &tv, NULL ) ;
   long now=( tv.tv_sec * 1000000000 ) + ( tv.tv_usec * 1000 ) ;

   /* Calc time difference and print. */
   delayMicroSec =  (now - start) / 1000000000.0;
   printf ("Took %5.2f seconds.n",delayMicroSec);

   return 0;
} 
#endif


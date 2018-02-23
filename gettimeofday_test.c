#include <sys/time.h>
#include <stdio.h>

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


/******************************************************************************
**
** frac-order.c
**
** Author: Tony Camuso
** November 3, 2011
** 
** This program will take a number of fractions input by the user and sort
** them by value, from the smallest to the largest. 
** 
** The fractions are entered Numerator first, then Denominator. Entry is 
** Terminated by entering zero at any time. Inputs are restrictd to decimal
** numbers in the range of 0 to 1000, inclusive.
**
** A lowest common denominator is found for all the fractions using a table
** driven version of the Euclidian algorithm that uses prime numbers to find
** a lowest common multiplier, so a prime number generator is employed. 
** See http://en.wikipedia.org/wiki/Least_common_multiple about half way down
** the page.  
**
** Once the fractions have been converted to their new common denominator, 
** they are sorted by their new numerator using qsort. 
** 
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>	// qsort	
#include <math.h>	// sqrt
#include <string.h>	// memset

#define ENTRIES 10
#define TRUE 1
#define FALSE 0

// A structure to represent a fraction
//
struct fraction {
	int inputNumerator;
	int inputDenominator;
	int lcdNumerator;	// Numerator for common denominator
	int lcdDenominator;	// Common denominator, same for every fraction
};

static char *ordinals[] = { 
	"First", "Second", "Third", "Fourth", "Fifth", 
	"Sixth", "Seventh", "Eighth", "Ninth", "Tenth"
};

/******************************************************
** Forward Function Prototype Declarations
******************************************************/
int  getFractions(struct fraction *fractions);
int  getNumber();
void printInputFractions(struct fraction *fractions, int count);
void printLcdFractions(struct fraction *fractions, int count);
int  getLowestCommonDenominator();
int  getNextPrime(int prime);
int  isPrime(int number);
void initLcdFractions(struct fraction *fractions, int count, int lcd);
int  compareLcdNumerator(const void *a, const void *b);

/////////////////////////////////////////////////////////////////////////////
//
// The main function and entry point
//
// Globals 
//
int main ()
{
	int count;				// number of fractions 
	int lcd; 				// lowest common multiple
	struct fraction fractions[ENTRIES];	// array of fraction structures
	
	printf("\nPut a list of up to %d fractions in ascending order\n\n", 
		ENTRIES);
	printf("Terminate input by entering \"0\"\n");

	memset(fractions, 0, ENTRIES * sizeof(struct fraction));

	count = getFractions(fractions);
	printf( "Number of fractions is %d, "
		"and the list of fractions is ...\n",count);
	printInputFractions(fractions, count);
	
	lcd = getLowestCommonDenominator(fractions, count);
	printf("The Lowest Common Denominator is %d.\n\n", lcd);
	
	initLcdFractions(fractions, count, lcd);
	printf("The list of Lowest Common Denominator fractions is ...\n");
	printLcdFractions(fractions, count);
	
	qsort(fractions, count, sizeof(struct fraction), compareLcdNumerator);
	
	printf( "The sorted list of the fractions "
		"from least to greatest is ...\n");
	printInputFractions(fractions, count);
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// getFractions
// 
// Obtains the numerator and denominator of each fraction and counts the
// number of fractions entered by the user. 
//
// Arguments : none
// Returns : none
//
// Global variables used
//	ordinals	- list of strings representing ordinal numbers
//	
int getFractions(struct fraction *fractions)
{
	int index;
	
	for (index = 0; index < ENTRIES; index++)
	{
		printf("%-7s Fraction\n", ordinals[index]);

		printf("\tNumerator   : "); 
		fractions[index].inputNumerator = getNumber();
		if(fractions[index].inputNumerator == 0)
			break;

		printf("\tDenominator : "); 
		fractions[index].inputDenominator = getNumber();
		if(fractions[index].inputDenominator == 0)
			break;

		putchar('\n');
	}
	putchar('\n');
	return index;
}

///////////////////////////////////////////////////////////////////////////////
//
// getNumber
//
// Returns the number input by the user. Checks for valid decimal number 
// input. If the input is not a valid decimal number, this functino will 
// continue to prompt the user for input until a valid decimal number is
// entered.
//
// Globals : none
// Arguments : none
// Returns integer conversion of user input
// 
int getNumber()
{
	int scanResult; 
	int number;
	char input[5];
	char *pInput = &input[0];
	
	while(1)
	{
		scanf("%4s", pInput);
		scanResult = sscanf(pInput, "%d", &number);
		
		// Make sure we got a valid number for input
		//
		if ((scanResult == 0) || (scanResult == EOF) || (number > 1000))
		{
			printf( "%s is not a valid decimal number "
				"from 0 to 1000.\n", pInput);
			printf("Try again : ");
			continue;
		}
		break;
	}
	return number;
}

///////////////////////////////////////////////////////////////////////////////
//
// printInputFractions
//
// Prints the list of fractions to the console
//
// Arguments : none
// Returns : none
// Globals :
//	fractions	- array of struct fraction
//	count		- number of fractions entered by user
//
void printInputFractions(struct fraction *fractions, int count)
{
	int index;
	
	for(index = 0; index < count; index++)
		printf( "%d/%d  ", 
			fractions[index].inputNumerator,
			fractions[index].inputDenominator );
	putchar('\n');
	putchar('\n');
}

///////////////////////////////////////////////////////////////////////////////
//
// printLcdFractions
//
// Prints the list of fractions to the console
//
// Arguments : none
// Returns : none
// Globals :
//	fractions	- array of struct fraction
//	count		- number of fractions entered by user
//
void printLcdFractions(struct fraction *fractions, int count)
{
	int index;
	
	for(index = 0; index < count; index++)
		printf( "%d/%d  ", 
			fractions[index].lcdNumerator,
			fractions[index].lcdDenominator );
	putchar('\n'); 
	putchar('\n');
}

///////////////////////////////////////////////////////////////////////////////
//
// getLowestCommonDenominator
//	Uses prime numbers to determine the least common denominator with
//	a table.
//
//	See http://en.wikipedia.org/wiki/Least_common_multiple about half
//	way down the page.  
// 
// Arguments
//	fractions	- pointer to the array of fraction structures
//	count		- the number of fractions entered by the user
//
// Returns 
//	lcd - lowest common denominator for all the fractions
//
int getLowestCommonDenominator(struct fraction *fractions, int count)
{
	int index;
	int lcd = 1;
	int primeNumber = 2; 	// Start with lowest prime number
	int lcdTableSize; 	// size in bytes of the lcmTable
	int *lcdTable;		// pointer to an array of integers used for 	
				// calculating the lowest common denominator

	lcdTableSize = count * sizeof(int);
	lcdTable = (int *)malloc(lcdTableSize);

	for(index = 0; index < count; index++)
		lcdTable[index] = fractions[index].inputDenominator;	

	// When the sum of the integers in the lcdTable is equql to their 
	// count, then all the integers in the lcdTable have been reduced
	// to the number 1.
	//
	while(sumTable(lcdTable, count) > count)
	{
		int match = FALSE;
		
		for(index = 0; index < count; index++)
		{
			if(lcdTable[index] % primeNumber != 0)
				continue;
			else
				lcdTable[index] /= primeNumber;
			
			if(match == FALSE)
			{
				lcd *= primeNumber;
				match = TRUE;
			}				
		}
		
		if(match == FALSE)
			primeNumber = getNextPrime(primeNumber);
	}
	
	return lcd;
}

///////////////////////////////////////////////////////////////////////////////
//
// sumTable
//	Sum the integers in the table passed by reference.
//
// Arguments
//	table	- pointer to an array of integers
//	count	- number of integers in the array
//
// Returns the sum of the integers in the array.
//
int sumTable(int *table, int count)
{
	int index;
	int sum = 0;
	
	for(index = 0; index < count; index++)
		sum += table[index];
	
	return sum;
}

///////////////////////////////////////////////////////////////////////////////
//
// getNextPrime
//
// Finds the next prime number up from the one passed. 
//
// Globals : none
// Arguments
//	prime - a prime number number
// Returns the next prime number
//
int getNextPrime(int prime)
{
	int next = prime;
	
	if(prime == 2)
		return 3;
		
	do {
		next += 2;
	} 
	while(isPrime(next) == FALSE);
	
	return next;
}

///////////////////////////////////////////////////////////////////////////////
//
// isPrime
//
// Determines whether a number is a prime number. 
//
// Globals : none
// Arguments
// 	number	- number to evaluate
// Returns : FALSE if not a prime, TRUE if number is prime
//
int isPrime(int number)
{
	int index;
	int prime = TRUE;
	
	if(number == 2)
		return TRUE;
		
	if(number %2 == 0 || number <= 1)
		return FALSE;
		
	for(index = 3; index <= sqrt(number) && prime == TRUE; index += 2)
		if(number % index == 0)
			prime = FALSE;
	
	return prime;
}

///////////////////////////////////////////////////////////////////////////////
//
// initLcdFractions
//
// Arguments
//	fractions	- pointer to an array of struct fraction
//	count		- number of fractions entered by user
//	lcd		- lowest common denominator
//
// Returns : none
//
void initLcdFractions(struct fraction *fractions, int count, int lcd)
{
	int i;
	
	for(i = 0; i < count; i++)
	{
		fractions[i].lcdNumerator = lcd * fractions[i].inputNumerator;
		fractions[i].lcdNumerator /= fractions[i].inputDenominator; 
		fractions[i].lcdDenominator = lcd;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// compareLcdNumerator
//
// This is a qsort callback function. qsort will pass void pointers to two 
// of the fraction structures for comparison by their LCD numerator. 
//
// Arguments
// 	a - the first fraction structure
//	b - the second fraction structure
//
int  compareLcdNumerator(const void *a, const void *b)
{
	const struct fraction *ia = (const struct fraction *)a;
	const struct fraction *ib = (const struct fraction *)b;
	return ia->lcdNumerator - ib->lcdNumerator;
}


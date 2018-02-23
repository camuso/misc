#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////////
//
// Create a class to contain the overloaded functions.
//

/********************************
** Class declaration
********************************/
//
// This is where the various members of the class are identified
// and declared.
//
class maxmin
{
public:
	maxmin();

	int getmax(int x, int y);
	int getmin(int x, int y);
	void printRange (int x, int y);
	void printRange (int x, int y, int& min, int& max);

private:
	int maxint;
	int minint;
};

/********************************
** Class Definition
********************************/
//
// This is where the various functions of the class are defined
//

// maxmin::maxmin - class constructor
//
// Simply print a message.
//
maxmin::maxmin()
{
	cout << endl << "Please type two integers." << endl;
}

// maxmin::getmax
//
// Returns the greater of the two integers given.
//
int maxmin::getmax(int x, int y)
{
	return x > y ? x : y;
}

// maxmin::getmin
//
// Returns the lesser of the two integers given.
//
int maxmin::getmin(int x, int y)
{
	return x < y ? x : y;
}

// maxmin::printRange - overloaded function
//
// int x - starting integer
// int y - ending integer
//
// This form of the function simply prints the sequence starting from
// the first number entered, regardless of whether it is the lesser or
// greater than the two.
//
void maxmin::printRange (int x, int y)
{
	while (x > y) {
		cout << x << endl;
		x--;
	}

	while (x <= y) {
		cout << x << endl;
		x++;
	}
}

// maxmin::printRange - overloaded function
//
// int x   - starting integer
// int y   - ending integer
// int min - will receive the value of the lesser number
// int max - will receive the value of the greater number
//
// This form of the function prints the sequence starting from the
// lesser number, regardless of whether it was entered first or last,
// and it also returns the lesser and greater numbers in the parameter
// list.
//
void maxmin::printRange (int x, int y, int& min, int& max)
{
	// The "this" pointer refers to this class, so it is pointing
	// to members of the class.
	//
	// Get the lesster and greater of the two integers passed.
	//
	this->minint = this->getmin(x, y);
	this->maxint = this->getmax(x, y);

	for(int i = minint; i <= maxint; i++)
		cout << i << endl;

	// Send back the lesser and greater integers to the caller
	// by initializing the last to members of the argument list.
	//
	min = this->minint;
	max = this->maxint;
}

///////////////////////////////////////////////////////////////////////
//
// The main program
//
int main ()
{
	// Declare some variables.
	//
	int x;		// First number to get from the user
	int y;		// Second number to get from the user
	int myMax;	// The greater of the two numbers
	int myMin;	// The lesser of the two numbers
	maxmin mm;	// Create an instance of the maxmin class

	// Get a number from the user.
	//
	cout << "Type x: ";
	cin >> x;
	cin.ignore();

	// Get another number from the user.
	//
	cout << "Type y: ";
	cin >> y;
	cin.ignore();

	// Call the first version of the maxmin class overloaded function
	// to output the integers from the first number input to the second.
	//
	cout << "From the first number to the second." << endl;
	mm.printRange(x, y);

	// Call the second version of the maxmin class overloaded function
	// to output the integers from the lesser of the two numbers to
	// the greater.
	//
	cout << endl << "Now from the smaller to the larger" << endl;
	mm.printRange(x, y, myMin, myMax);
	cout << "The larger number is: " << myMax << endl;
	cout << "the smaller number is: " << myMin << endl << endl;

	return 0;	// Return good status
}

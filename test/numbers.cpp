#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;

// getIntegerInRange - prompts the user for an integer and checks to
//                     see that it is within the prescribed range.
//
// int min - minimum value of num
// int max - maximum value of num
//
// Returns a number within the range defined  by min and max.
//
int getIntegerInRange(int min, int max)
{
    int num;
    bool isOutOfRange;

    // Keep asking for input until you get a number that's within range.
    //
    do {
        cout << "Please type an integer between " << min <<" and "
             << max << " : ";
        cin >> num;
        cin.ignore();

        // IF the number is less than the "min" value
        // OR the number is greater than the "max" value
        // Then it is out of range, so "isOutOfRange" will be true.
        //
        isOutOfRange = ((num < min) || (num > max));

        if(isOutOfRange)
            cout << "ERROR: The number you typed is out of range." << endl;

    } while (isOutOfRange);

    return num;
}


int main ()
{
    // Define our minimum and maximum numbers and the constant to add to
    // them for our output.
    //
    static const int minNum = 0;
    static const int maxNum = 20;
    static const int addend = 17;
    int num;

    cout << "We will ask you for a number between " << minNum
         << " and " << maxNum << endl
         << "Then we will add " << addend
         << " to your input and print the result on the screen." 
         << endl << endl;

    num = getIntegerInRange(minNum, maxNum);

    cout << num << " + " << addend << " = " << num + addend << endl << endl;

    return 0;
}


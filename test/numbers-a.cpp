#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;

int main ()
{
    // Define our minimum and maximum numbers and the constant to add to
    // them for our output.
    //
    const int min = 0;
    const int max = 20;
    const int addend = 17;

    int num;
    bool isOutOfRange;

    cout << "We will ask you for a number between " << min
         << " and " << max << endl
         << "Then we will add " << addend
         << " to your input and print the result on the screen."
         << endl << endl;

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

    cout << num << " + " << addend << " = " << num + addend << endl << endl;

    return 0;
}


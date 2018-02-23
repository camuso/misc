#include <stdio.h>
#include <iostream>

using namespace std;

#define COUNT 10

int power(int mantissa, int exponent)
{
    int retVal = mantissa;

    for(int i = 1; i < exponent; ++i)
         retVal *= mantissa;

    cout << mantissa << " to the " << exponent 
         << " power is: " << retVal << endl;

    return retVal;
}


main()
{
    const int start = 5;   // Number of required inputs
    int exp = 3;
    int num;
    int sum = power(start, exp);
    int firstNumber;

    cout << sum << endl;    
    cout << "Please give us a number: ";

    cin >> num;
    cin.ignore();
    firstNumber = num;

    while(num != start) {
     
        sum += power(num, exp);

        cout << "sum: " << sum << endl << endl;
 
        if(num > 5)
            num--;
        else
            num++;

        //num = num > 5 ? num-- : num++;
    }

    cout << "You entered " << num << endl
         << "The sum of the cubes from " << firstNumber << " to " << start  
         << " is: " << sum << endl; 
}


#include <stdio.h>
#include <iostream>

using namespace std;

#define COUNT 10

main()
{
    const int count = 10;   // Number of required inputs
    int num;
    int sum = 0;
    
    int smallestNumber = 0;
    bool firstNumber = true;    

    char str1[] = "ABCD";
    char str2[] = "wxyz";

    char *ptr1 = & str1[0];
    char *ptr2 = & str2[0];

    cout << "str1: " << str1 << endl;
    cout << "str2: " << str2 << endl << endl;

    while(*ptr1 != '\0')
        *ptr1++ = *ptr2++;

    cout << "str1: " << str1 << endl;
    cout << "str2: " << str2 << endl;

    for(int i = 0; i < count; ++i) {
        cout << i << ". Please give us a number: ";

        cin >> num;
        cin.ignore();

        if(num < 0) {
            cout << "NO NEGATIVE NUMBERS!" << endl;
            i--;
            continue;
            // break;
        }

        sum += num;

#if 0
        if(num < smallestNumber)
            smallestNumber = num;
#endif

        if(firstNumber || (num < smallestNumber)) {
            smallestNumber = num;
            firstNumber = false;
        }
    }

    cout << endl << "You have entered " << count << " numbers, "
         << "and their sum is: " << sum << endl 
         << "and the smallest number was: " << smallestNumber << endl << endl;
}


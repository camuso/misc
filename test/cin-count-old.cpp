
#include <stdio.h>
#include <iostream>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>

using namespace std;

string usage =
"\n"
"This program receives a string of numeric input from the user and counts\n"
"the number of integers that are the same in a given sequence. It does not\n"
"count the number that are the same throughout the whole line.\n"
"\n"
"Numbers must be separated by a non-digit character.\n"
"\n"
"The program continues to ask for an input line until it encounters a \'q\'\n"
"or \'Q\' in the input line. At that point, the program will print the\n"
"last group of identical integers and exit.\n"
"\n";

// printCount - print the number of times "value" appeared
//
// newVal       - the new value we encountered in the input
// currentValue - passed by reference, because it will be set to the newVal
// count        - number of times currentValue appeared. Passed by reference
//                because it will be set to 1.
//
void printCount(int newVal, int& currentValue, int& count)
{
    cout << currentValue << " occurs " << count << " times" << endl;
    currentValue = newVal;  // remember the new value
    count = 1;              // reset the counter
}

bool getAnInt(int& value, string instr, string::iterator& it)
{
    char *cptr;     // We need a character pointer.
    
    // Skip all the non-digit characters.
    //
    // This statement advances the iterator past any leading
    // character that is not a digit, as long as we have not
    // reached the end of the string (in.end()).
    //
    // If the character addressed by the iterator is not a digit,
    // then the iterator is advanced to the next character by the
    // post-increment ++ operator.
    //
    // The lone semicolon is known as an "empty statement", since
    // there is nothing to do beyond what is done in the while()
    // declaration.
    //
    while(( it != instr.end()) && ( ! isdigit(*it++)))
        ;
        
    // If we have reached the end of the string, then return false, 
    // as we did not find an integer. 
    //
    if(it == instr.end())
        return false;
      
    // If we get here, it's because we've found a digit. However, the
    // iterator is pointing past the digit, because we incremented the
    // iterator even after we found a digit, so we need to back up.
    //
    --it;
    
    // Set the char pointer to the address of the character being
    // pointed at by the iterator of the 'in' string. The iterator
    // contains the address of a character in the string. However,
    // the iterator is more than just a pointer, it is an iterator
    // type. We cannot directly convert an iterator type to char 
    // pointer, so we take the address of (&) the character being
    // pointed at by the iterator (*it).
    //
    cptr = &(*it); 
    // cout << "cptr: " << cptr << endl;
    
    // At this point, cptr should be pointing at a digit, so let's
    // convert whatever digit characters may be there into an
    // integer.
    //
    value = atoi(cptr);
    
    // Now we need to advance the iterator beyond whatever digits we
    // found.
    //
    while(( it != instr.end()) && (isdigit(*it++)))
        ;

    --it;
     return true;
}


// isNumber - returns true if the string contains a number, else false
//
// const string& str - declared const to protect the contents of the string
//
bool isNumber(const string& str)
{
    // Get an iterator and initialize it to the beginning of the string.
    // Must use const_iterator since the string is a const.
    //
    string::const_iterator it = str.begin();
    
    // Look for a digit in the string
    //
    while (it != str.end() && isdigit(*it))
        ++it;
    
    // If the string is NOT empty, and it is NOT at the end, then return
    // true, else return false.
    //
    return !str.empty() && it == str.end();
}

int main ()
{
    // currVal is the number we're counting; we'll read new values into val

    int currVal = 0;
    int val = 0;
    int cnt = 1;        // initialize to '1' becase we expect at least 1  
    string inStr;       // The input string
    
    // Instruct the user.
    //
    cout << usage;
    
    // Read a line of user input, and put it into the string named 'in'
    //
    while(getline(cin, inStr)) {

        // Initialize the 'cnt' to start with '1', because we expect to 
        // find at least one.
        //
        int cnt = 1; 
        
        string buffer;          // Temporary buffer for the tokenizing process
        vector<string> tokens;  // A vector to contain the tokenized string
        stringstream ss(inStr); // Put the input string into a stream
        
        // Tokenize the input string.
        // The input string inStr has been inserted into a stream, ss.
        // The extraction operator >> will extract strings within the
        // stream on whitespace boundaries. For example, the following
        // string, "this is a string", is actually made up of four strings
        // separated by white space: "this", "is", "a", "string". The process
        // of breaking a string up into parts separated by a delimiter, in
        // this case white space, is called "tokenizing". 
        //
        while(ss >> buffer)
            tokens.push_back(buffer);
            
        // Initialize an iterator to walk through the tokens.
        //
        vector<string>::iterator vit = tokens.begin();
        
        // Get the first number and store it into currVal
        //
        if(isNumber(*vit))
            currVal = atoi((*vit).c_str());
        
        // Point to the next string in the vector of tokens.
        //
        ++vit;
        
        // Get the rest of the numbers.
        //
        while(vit != tokens.end()) {
        
            if(isNumber(*vit)
                val = atoi((*vit).c_str());
                        
            // If the values are the same, then increment the cnt, otherwise
            // print the number of digits that were the same as the previous
            // ones.
            //
            if(val == currVal)
                ++cnt;
            else 
                printCount(val, currVal, cnt);
            
            // Increment the iterator to the next string in the vector of 
            // tokens.
            //
            ++vit;
        }
        // end while
    } 
    // end while

   return 0;
}
// end main



#include <iostream>
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
"The program will continue to seek input until the user types \'q\' or \'Q\'.\n"
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

// isNumber - returns true if the string contains a number, else false
//
// const string& str - declared const to protect the contents of the string
//
bool isNumber(const string& str)
{
    // Get an iterator and initialize it to the beginning of the string.
    // Must use const_iterator, because the string is a const.
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
    
    // Read a line of user input, and put it into the std::string named 'inStr'
    //
    while(getline(cin, inStr)) {

        // If the string is empty, try again.
        //
        if(inStr.empty()) {
            cout << "Please give us some input." << endl;
            continue;
        }
        
        // Initialize the 'cnt' to start with '1', because we expect to 
        // find at least one.
        //
        int cnt = 1; 
        
        string buffer;          // Temporary buffer for the tokenizing process
        vector<string> tokens;  // A vector to contain the tokenized strings
        stringstream ss(inStr); // Put the input string into a stream
        
        // Tokenize the input string.
        // The input std::string inStr has been inserted into a stream, ss.
        // The extraction operator >> will extract strings within the
        // stream on whitespace boundaries. For example, the following
        // string, "this is a string", can be managed as though it were
        // made up of four strings separated by white space: 
        //      "this"  "is"  "a"  "string". 
        // The process of breaking a string up into strings separated by a
        // delimiter is called "tokenizing". In this case, the delimiter
        // is simply whitespace.
        //
        while(ss >> buffer)
            tokens.push_back(buffer);
            
        // Initialize an iterator to walk through the vector of std::string
        // tokens. The iterator points to a std::string object in the vector
        // of std::string tokens.
        //
        vector<string>::iterator vit = tokens.begin();
        
        // If the first character in the string is 'q' or 'Q', then the
        // user wants to quit, so break out of this loop.
        //
        if((*vit).find_first_of("qQ") != string::npos)
            break;
        
        // If the first std::string in the vector of std::string tokens
        // has a number in it, convert the number to an int, and store
        // it in currVal.
        //
        // vit is an iterator that points to a std::string in the
        // vector containing the std::string tokens.
        //
        // (*vit) is the std::string itself.
        //        
        // (*vit).c_str() returns a "C string" array of characters
        // which is needed by the atoi() function.
        //
        // The atoi() function converts the numeric ascii characters in
        // a C string into an integer and returns the integer.
        //
        if(isNumber(*vit))
            currVal = atoi((*vit).c_str());
        
        // Point to the next std::string in the vector of std::string tokens.
        //
        ++vit;
        
        // While the vit iterator is not pointing to the end of the tokens
        // vector containing the tokenized std::strings ...
        //
        while(vit != tokens.end()) {
        
            // If this std::string contains a number, then process it.
            //
            if(isNumber(*vit)) {

                // Convert the std::string containing the number into an
                // integer. 
                //
                val = atoi((*vit).c_str());
                
                // If the values are the same, then increment the cnt, 
                // else print the number of digits that were the same as the
                // previous ones.
                //
                if(val == currVal)
                    ++cnt; 
                else
                    printCount(val, currVal, cnt);
            }
                
            // Increment the iterator to the next std::string in the vector of 
            // std::string tokens.
            //
            ++vit;
        }
        // end while
        
        // Print the last one.
        //
        printCount(val, currVal, cnt);
    } 
    // end while

    return 0;
}
// end main


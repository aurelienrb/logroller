#include <iostream>
#include <string>
#include <sstream>
using namespace std;

int main(int argc, char **argv)
{
    // if we receive an address to print on the commande line
    if (argc == 3 && string(argv[1]) == "-print")
    {
        // get the address to print
        istringstream iss(argv[2]) ;
        void *addr;
        if (iss >> addr)
        {
			// print the given address as a char *
            cout << (const char *)addr << endl;
        }
    }
    else
    {
		// print the address of the message (not the message itself)
        cout << (void *)("Hello World!") << endl;
    }
}

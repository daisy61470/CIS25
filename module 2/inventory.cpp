#include <iostream>
using namespace std;

int main() {
    // cout prints to the screen; the line below prints the greeting
    cout << "Hello Inventory System!\n";

    cout << "\n--- Escape Character Demo ---\n";
    cout << "Tab between columns:\tItem\tQty\tCost\n";
    cout << "New line after this sentence.\n";
    cout << "A quote mark: \" and a backslash: \\\n";

    cout << "\n--- Data Type Sizes ---\n";
    cout << "Size of int:   " << sizeof(int)   << " bytes\n";
    cout << "Size of float: " << sizeof(float) << " bytes\n";
    cout << "Size of char:  " << sizeof(char)  << " bytes\n";

    return 0;
}

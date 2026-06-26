
#include <iostream>
#include <string>
#include <iomanip>   // needed for fixed and setprecision
using namespace std;

int main() {
    // Declare variables using clear, camelCase naming conventions
    string itemName;
    int quantity;
    float cost;

    // Prompt the user and read each value
    cout << "Enter item name: ";
    cin >> itemName;

    cout << "Enter quantity: ";
    cin >> quantity;

    cout << "Enter cost: ";
    cin >> cost;

    // Calculate the total
    float totalCost = quantity * cost;

    // Display money with exactly two decimal places
    cout << fixed << setprecision(2);
    cout << "Total cost for " << itemName << ": $" << totalCost << endl;

    return 0;
}

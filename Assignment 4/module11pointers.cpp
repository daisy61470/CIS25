// CIS 25 - Assignment 4, Module 11
// C++ Basics: Pointers
// Daisy Schall
// Program: Patient Application using a pointer that stores the
// address of a regular variable (no dynamic memory).

#include <iostream>
#include <string>
using namespace std;

// 1. Create a struct called Patient with id, name, and age
struct Patient {
    int id;
    string name;
    int age;
};

int main() {
    // 2. Create a regular Patient variable
    Patient patient1;

    // 3. Create a pointer that stores the address of the patient using &
    Patient* p = &patient1;

    // 4. Assign values using the regular variable
    patient1.id = 305;
    patient1.name = "Alex Rivera";
    patient1.age = 42;

    // 5. Display patient information using the pointer (->)
    cout << "--- Patient Record ---" << endl;
    cout << "ID: " << p->id << endl;
    cout << "Name: " << p->name << endl;
    cout << "Age: " << p->age << endl;

    return 0;
}

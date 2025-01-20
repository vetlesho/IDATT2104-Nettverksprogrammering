#include <iostream> // input and output 
using namespace std; // names for objects and variables


void myFunction(string navn) {
    cout << "\nNå erre ny funksjon på gang. navnet er " << navn;
}

int inputFunction() {
    int x, y;
    int sum;
    cout << "Skriv inn x-verdi: ";
    cin >> x;
    cout << "Skriv inn y-verdi: ";
    cin >> y;
    sum = x + y;
    cout << "Summen ble: " << sum <<"\n";
    return 0;
}

int main () {
    cout << "Hello world! \n";

    int num = 12;
    int a = 2, b = 3;
    cout << num << "\n";
    cout << b * a << "\n"; 

    const int minutesPerHour = 60;
    cout << "this variable is a const cus it is unlikely to change: " << minutesPerHour <<"\n";
    // minutesPerHour = 61; // will not work

    bool imCool = false;
    if (imCool)
    {
        cout << "Helt riktig, jeg er kul";
    } else {
        cout << "not cool dode";
    }
    

    int dayx = 2;
    switch (dayx) {
        case 1:
            cout << "Mandag";
            break;
        case 2: 
            cout << "Tirsdag";
            break;
        default:
            cout << "nå erre noe feil";
            break;
    }
    cout << "\n";

    for (int i = 0; i < 5; i++)
    {
        cout << i << "\n";
    }

    string cars[3] = {"volvo", "bmw", "ford"};
    cout << cars[0] << "\n";

    string food = "Pizza";
    cout << food;
    cout << " På plass: " << &food << "\n";

    string burger = "Hamburger"; // variable declaration
    string* ptr2 = &burger; // pointer declaration
    
    // Reference: output the memory adress of food with the pointer
    cout << ptr2 << "\n";
    // Dereference: output the value of food with the pointer
    cout << *ptr2 << "\n";

    myFunction("Vetle");
    inputFunction();

    return 0;
}
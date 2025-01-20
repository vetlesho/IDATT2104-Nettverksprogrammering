#include <iostream>
#include <thread>
#include <vector> //vector is like a resizable array, stores elements of the same data type.
#include <mutex>

using namespace std;

// compile with threads, then run
// g++ -std=c++11 -o primenumber primenumber.cpp
// ./primenumber

mutex mtx;

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i < n; i++){
        //cout << "Checking " << n << " % " << i << endl;
        if (n % i == 0) return false;
    } 
    return true;
}

void findPrimes(int min, int max, vector<int> &primeNumbers) {
    vector<int> localPrimes;
    for (int i = min; i < max; i++){
        if (isPrime(i)){
            localPrimes.push_back(i);
        }
    }

    lock_guard<mutex> lock(mtx);
    primeNumbers.insert(primeNumbers.end(), localPrimes.begin(), localPrimes.end());
}

void sortPrimes(vector<int> &primeNumbers) {
    sort(primeNumbers.begin(), primeNumbers.end());
}

int main() {
    int min, max, numThreads;
    cout << "Write the lowest number: ";
    cin >> min;
    cout << "Write the highest number: ";
    cin >> max;
    cout << "How many threads do u want to use: ";
    cin >> numThreads;

    if (min >= max || min <= 0 || max <= 0 || numThreads <= 0 || numThreads > 1000) {
        cout << "Invalid input";
        return 0;
    } 

    vector<thread> threads;
    vector<int> primeNumbers;

    int range = max - min + 1;
    int chunkSize = range / numThreads;
    int start = min;

    for (int i = 0; i < numThreads; ++i) {
        int end = start + chunkSize - 1;
        if (i == numThreads - 1) {
            end = max; // Last thread takes the remainder
        }

        threads.emplace_back(findPrimes, start, end, ref(primeNumbers));
        start = end + 1;
    }

    for (auto &thread : threads) {
        thread.join();
    }

    for (int prime : primeNumbers) {
        cout << prime << " ";
    }
    cout << endl;

    sortPrimes(primeNumbers);
    for (int prime : primeNumbers) {
        cout << prime << " ";
    }
    
    cout << "\nNumber of prime numbers: " << primeNumbers.size() << endl;

    return 0;
}
#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

using namespace std;

/**
 * @brief Class implementing the spinlock using atomic function test and set.
 * 
 * Method acquire checks if lock is available and take it; or busy waits.
 * Method release, releases lock for other thread to take it and enter cr.
 */
class SpinLock {
public:
  SpinLock() : flag_(ATOMIC_FLAG_INIT) {}

  void acquire() {
    while (flag_.test_and_set(memory_order_acquire)) {
      cout<<"Waiting for lock release"<<endl;
      // busy-wait
    }
  }

  void release() {
    flag_.clear(memory_order_release);
  }

private:
  atomic_flag flag_;
};

// GLOBAL VARIABLES OF THE PROGRAM
int total = 0;
SpinLock sum_lock;

/**
 * @brief Function that sums array in distributed manner. 
 * Each thread will do a local sum, then update the total value.
 * This minimizes the time each thread stays in cr.
 * 
 * @param arr Complete array which will be sum up.
 * @param start Start of the slice of array this thread will sum.
 * @param end End of the slice of array this thread will sum.
 */
void sumArray(signed char* arr, int start, int end){
    int localTotal = 0;
    for (int i = start; i < end; i++) {
      localTotal += arr[i];
    }
    // BEGIN OF CRITICAL REGION
    sum_lock.acquire();
    total += localTotal;
    sum_lock.release();
    // END OF CRITICAL REGION
}

/**
 * @brief Creates array of random num, creates threads and splits array between them.
 * Does all the testing for the different number of threads and size of arr
 */
int main() {

    int arrExpoArraySize[3] = {7,8,9}; 
    int arrNumThreads[9] = {1,2,4,8,16,32,64,128,256};

    // Create and open a text file
    ofstream MyFile("results.txt");

    for (int a = 0; a < 3; a++){
        int kExpoArraySize = arrExpoArraySize[a];
        for (int t = 0; t < 9; t++){
            int kNumThreads = arrNumThreads[t];
            double total_time_taken = 0;
            for (int m = 0; m < 10; m++){
                cout << "array size: 10^" << kExpoArraySize;
                cout << ", number of threads: " << kNumThreads <<endl;

                // Creates array and fills it with random numbers
                int kArraySize = pow(10, kExpoArraySize);
                signed char* arr = new signed char[kArraySize];
                srand(time(NULL));
                for (int i = 0; i < kArraySize; i++) {
                    int x = rand() % 201 - 100;
                    signed char c = x;
                    arr[i] = x;
                }
                // Gets expcted res to compare
                int exp = 0;
                for (int i = 0; i < kArraySize; i++) {
                    exp+=arr[i];
                }
                cout<<"The expected result is: " << exp <<endl;

                // Create threads
                vector<thread> threads(kNumThreads);
                auto start = chrono::high_resolution_clock::now();
                int chunkSize = kArraySize / kNumThreads;
                // Splits array and call function.
                for (int i = 0; i < kNumThreads; i++) {
                    int start = i * chunkSize;
                    int end = (i == kNumThreads - 1) ? kArraySize : (i + 1) * chunkSize;
                    threads[i] = thread(sumArray, arr, start, end);
                }
                // Waits for threads to finish.
                for(auto& thread : threads){
                    thread.join();
                }
                auto end = chrono::high_resolution_clock::now();

                // Print results and exec time.
                cout << "Total: " << total << endl;
                if (total!=exp){
                    cout<<"ERROR: Total obtained does not match the expected value!" <<endl;
                }
                chrono::duration<double, milli> time_taken = end - start;
                cout << "Time taken by program is : "  << time_taken.count() << " ms" << endl << endl;
                total_time_taken += time_taken.count();
                // Clean
                delete[] arr;
                total = 0.0;
            }
            // Write to the file
            MyFile << total_time_taken/10 <<" ";
            total_time_taken = 0.0;
        }
        MyFile << endl;
    }
    // Close the file
    MyFile.close();
    return 0;
}
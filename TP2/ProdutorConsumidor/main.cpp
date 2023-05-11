#include <cstdlib>
#include <time.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <semaphore.h>
#include <math.h>

using namespace std;

#define MAX_CONSUMED 100000 // 10^5

class SharedMemory {
    public:
        int *vec;
        int vec_size;
        // Index of the oldest produced item (consumer will pick from here)
        int start_index = 0;
        // Index of the newest produced item (producer will put here)
        int end_index = 0;
        SharedMemory(int N){
            vec_size = N;
            vec = (int*) calloc(N, sizeof(int));
        }    
};

SharedMemory *sharedMemory;
sem_t buffer_empty;
sem_t buffer_full;
sem_t mutex;
bool running;
int totalConsumed;
vector<int> ocupacao_buffer;
int produz_consumo = 0;

/**
 * @brief checks if n in prime in O(n)
 * 
 * @param n number to be checked if prime
 * @return true if n is prime
 * @return false if n is not prime
 */
bool is_prime(unsigned int n) {
  if (n == 0 || n == 1) {
    return false;
  }

  for (int i = 2; i <= n / 2; ++i) {
    if (n % i == 0) {
      return false;
    }
  }

  return true;  
}

/**
 * @brief thread function to produce a number between 1 and 10^7 and add in the shared memory buffer.
 * 
 */
void producer_func() {
    int produced;
    while(running) {
        produced = 1 + rand()%((int)pow(10,7));
        sem_wait(&buffer_empty);
		sem_wait(&mutex);
        if (running) {
            sharedMemory->vec[sharedMemory->end_index] = produced;
            sharedMemory->end_index = (sharedMemory->end_index + 1)%sharedMemory->vec_size;
            produz_consumo++;
            ocupacao_buffer.push_back(produz_consumo);
        }
        sem_post(&mutex);
		sem_post(&buffer_full);
    }
}

/**
 * @brief thread function to consume the number from the shared memory buffer and say if is prime or not.
 * 
 */
void consumer_func() {
    int item;
    while(running) {
        sem_wait(&buffer_full);
		sem_wait(&mutex);

        if(totalConsumed == MAX_CONSUMED) {
            running = false;
        } else {
            item = sharedMemory->vec[sharedMemory->start_index];
            sharedMemory->vec[sharedMemory->start_index] = 0;
            sharedMemory->start_index = (sharedMemory->start_index + 1)%sharedMemory->vec_size;
            totalConsumed++;
            produz_consumo--;
            ocupacao_buffer.push_back(produz_consumo);
        }

        sem_post(&mutex);
		sem_post(&buffer_empty);

        if(running) {
            if(is_prime(item)){
                // cout << "[" << totalConsumed <<"] The number " << item << " is prime." << endl;
            } else {
                // cout << "[" << totalConsumed <<"] The number " << item << " is not prime." << endl;
            }
        }
    }
}

int main(int argc, char const *argv[]) {

    int Np, Nc, N;
    struct timespec start, end;
    double time_taken;
    srand(time(NULL));

    double total_time_taken = 0.0;
    for (int m = 0; m < 10; m++){
        running = true;
        totalConsumed = 0;
        Np=1, Nc=8, N=10;

        // Create Shared Memory
        sharedMemory = new SharedMemory(N);

        // Semaphores Inicialization
        if (sem_init(&buffer_empty, 0, N) || sem_init(&buffer_full, 0, 0) || sem_init(&mutex, 0, 1)) {
            cout << "Error trying to initializate the semaphore." << endl;
        }

        // Create threads
        vector<thread> threadsProducer(Np);
        vector<thread> threadsConsumer(Nc);
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Producer Threads creation
        for (int i = 0; i < Np; i++) {
            threadsProducer[i] = thread(producer_func);
        }

        // Consumer Threads creation
        for (int i = 0; i < Nc; i++) {
            threadsConsumer[i] = thread(consumer_func);
        }

        // Wait for all consumer threads to finish
        for(auto& thread : threadsConsumer){
            thread.join();
        }

        // Wait for all producer threads to finish
        for(auto& thread : threadsProducer){
            thread.join();
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken = (end.tv_sec - start.tv_sec);
        time_taken += (end.tv_nsec - start.tv_nsec) / 1000000000.0;
        total_time_taken+=time_taken;
        cout << "Tempo ["<< m <<"]: " << time_taken << endl;
    }
    cout << "Tempo Medio: " << total_time_taken/10 << endl;

    // Destroy Semaphores
	sem_destroy(&buffer_empty);
	sem_destroy(&buffer_full);
	sem_destroy(&mutex);
    
    return 0;
}
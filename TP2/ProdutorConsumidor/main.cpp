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
void producer_func(bool count_ocupacao) {
    int produced;
    while(running) {
        produced = 1 + rand()%((int)pow(10,7));
        sem_wait(&buffer_empty);
		sem_wait(&mutex);
        if (running) {
            sharedMemory->vec[sharedMemory->end_index] = produced;
            sharedMemory->end_index = (sharedMemory->end_index + 1)%sharedMemory->vec_size;
            if(count_ocupacao){
                produz_consumo++;
                ocupacao_buffer.push_back(produz_consumo);
            }
        } else {
            for (int ii = 0; ii < 9 ; ii++){
                sem_post(&buffer_full);
            }
        }
        sem_post(&mutex);
		sem_post(&buffer_full);
    }
}

/**
 * @brief thread function to consume the number from the shared memory buffer and say if is prime or not.
 * 
 */
void consumer_func(bool count_ocupacao) {
    int item;
    while(running) {
        sem_wait(&buffer_full);
		sem_wait(&mutex);

        if(totalConsumed == MAX_CONSUMED) {
            running = false;
            cout << "Chegou limite de consumidor." << endl;
            for (int ii = 0; ii < 9 ; ii++){
                sem_post(&buffer_empty);
            }
        } else {
            item = sharedMemory->vec[sharedMemory->start_index];
            sharedMemory->vec[sharedMemory->start_index] = 0;
            sharedMemory->start_index = (sharedMemory->start_index + 1)%sharedMemory->vec_size;
            totalConsumed++;
            if(count_ocupacao){
                produz_consumo--;
                ocupacao_buffer.push_back(produz_consumo);
            }
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

    int arrNumOfN[4] = {1, 10, 100, 1000}; 
    int arrThreadCombination[7][2] = {{1,1}, {1, 2}, {1, 4}, {1, 8}, {2, 1}, {4, 1}, {8, 1}};

    int Np, Nc, N;
    struct timespec start, end;
    double time_taken;
    srand(time(NULL));

    // Create and open a text file
    ofstream MyFile("results.txt");

    for(int idxN = 0; idxN < 4; idxN++) {
        N = arrNumOfN[idxN];

        for(int idxCT = 0; idxCT < 7; idxCT++) {
            Np = arrThreadCombination[idxCT][0];
            Nc = arrThreadCombination[idxCT][1];

            // Create and open a text file
            ofstream MyFileOcupation("ocupation_"+to_string(N)+"_"+to_string(Np)+"_"+to_string(Nc)+".txt");

            double total_time_taken = 0.0;
            for (int m = 0; m < 10; m++){
                running = true;
                totalConsumed = 0;
                produz_consumo = 0;

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
                    threadsProducer[i] = thread(producer_func, m==5);
                }

                // Consumer Threads creation
                for (int i = 0; i < Nc; i++) {
                    threadsConsumer[i] = thread(consumer_func, m==5);
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
                //cout << "Tempo ["<< m <<"]: " << time_taken << endl;
            }
            cout << "N: " << N << " | Np: " << Np << " | Nc: " << Nc << endl;
            cout << "Tempo Medio: " << total_time_taken/10 << endl;

            for (int idx_ocup = 0; idx_ocup < ocupacao_buffer.size(); idx_ocup++) {
                MyFileOcupation << ocupacao_buffer[idx_ocup] << " ";
            }
            MyFileOcupation << endl;

            // Write to the file
            MyFile << total_time_taken/10 <<" ";
            MyFileOcupation.close();

            ocupacao_buffer.clear();
        }
        MyFile << endl;
    }
    // Close the file
    MyFile.close();

    // Destroy Semaphores
	sem_destroy(&buffer_empty);
	sem_destroy(&buffer_full);
	sem_destroy(&mutex);
    
    return 0;
}
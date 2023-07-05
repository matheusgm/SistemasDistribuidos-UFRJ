#include <iostream>
#include <thread>
#include <chrono>

// Function to be executed in the child thread
void threadFunction() {
    // Perform some task in the child thread
    while (true) {
        std::cout << "Child thread is running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    // Create a child thread
    std::thread childThread(threadFunction);

    // Wait for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Terminate the child thread
    childThread.join();

    std::cout << "Child thread has been terminated." << std::endl;

    return 0;
}

#include <bits/stdc++.h>
#include <unistd.h>
#include <filesystem>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

void runProgram(string path, string id, int n, int r, int k)
{
    // Getting filename for -o
    string base_filename = path.substr(path.find_last_of("/\\") + 1);
    string::size_type const p(base_filename.find_last_of('.'));
    string file_without_extension = base_filename.substr(0, p);

    // Compiling c++ files
    string command = "g++ -O2 -g -std=c++17 -pthread -o " + file_without_extension + id + ".out ";
    command = command + path;
    cout << "Compiling file using " << command << endl;
    int systemRetComp = system(command.c_str());
    if (systemRetComp == -1)
    {
        // The system method failed
    }

    // Running compiled files
    cout << "Running file " + file_without_extension + id + ".out\n";
    string run = "./" + file_without_extension + id + ".out " + " " + to_string(n) + " " + to_string(r) + " " + to_string(k);
    int systemRetRun = system(run.c_str());
    if (systemRetRun == -1)
    {
        // The system method failed
    }

    // Removing Executables
    string del = file_without_extension + id + ".out";
    remove(del.c_str());
    cout << "File deleted.\n";
    return;
}

int main(int argc, char const *argv[])
{
    // Parsing arguments
    int n = atoi(argv[1]); // number of processes
    int r = atoi(argv[2]); // times each process writes in file
    int k = atoi(argv[3]); // sleep time

    // Create and open result text file
    ofstream MyResult("resultado.txt");
    MyResult.close(); // Close file

    // Path for coordinator and process code
    fs::path currentPath = fs::current_path();
    string program1Path = (currentPath / "coordinator.cpp").string();
    string program2Path = (currentPath / "process.cpp").string();

    // Create threads to run each program
    thread thread_coordinator(runProgram, program1Path, "", n, r, k);
    sleep(2);
    vector<thread> thread_process(n);
    for (int i = 0; i < n; i++)
    {
        thread_process[i] = thread(runProgram, program2Path, to_string(i), n, r, k);
    }

    // Wait for threads to finish
    thread_coordinator.join();
    for (auto &thread : thread_process)
    {
        thread.join();
    }

    std::cout << "\nDone." << std::endl;

    return 0;
}

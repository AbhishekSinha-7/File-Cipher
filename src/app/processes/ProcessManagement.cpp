#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <windows.h>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include "../encryptDecrypt/Cryption.hpp"
#include <Task.hpp>

class ProcessManagement {
public:
    ProcessManagement(size_t max_threads = 4);
    ~ProcessManagement();
    bool submitToQueue(std::unique_ptr<Task> task);
    void executeTasks();

private:
    std::queue<std::unique_ptr<Task>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable taskCV;
    std::vector<std::thread> workerThreads;
    bool shutdownFlag = false;

    void workerThreadFunction();
    void executeTask(std::unique_ptr<Task> task);
};

ProcessManagement::ProcessManagement(size_t max_threads) {
    for (size_t i = 0; i < max_threads; ++i) {
        workerThreads.emplace_back(&ProcessManagement::workerThreadFunction, this);
    }
}

ProcessManagement::~ProcessManagement() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        shutdownFlag = true;
    }
    taskCV.notify_all(); // Wake all threads
    
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(std::move(task));
    }
    taskCV.notify_one(); // Signal that a new task is available
    return true;
}

void ProcessManagement::workerThreadFunction() {
    while (true) {
        std::unique_ptr<Task> task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskCV.wait(lock, [this] {
                return shutdownFlag || !taskQueue.empty();
            });
            
            if (shutdownFlag && taskQueue.empty()) {
                return; // Exit thread on shutdown
            }
            
            if (!taskQueue.empty()) {
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
        }
        
        if (task) {
            executeTask(std::move(task));
        }
    }
}

void ProcessManagement::executeTask(std::unique_ptr<Task> task) {
    std::cout << "Executing task: " << task->toString() << std::endl;
    executeCryption(task->toString());

#ifdef _WIN32
    std::string taskStr = task->toString();
    std::string command = "cryption.exe " + taskStr;

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
    std::wstring wcommand(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wcommand[0], size_needed);

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(
            NULL, &wcommand[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Error: CreateProcess failed (" << GetLastError() << ")" << std::endl;
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    pid_t childProcessToRun = fork();
    if (childProcessToRun == 0) {
        std::string taskStr = task->toString();
        char* args[3];
        args[0] = strdup("./cryption");
        args[1] = strdup(taskStr.c_str());
        args[2] = nullptr;
        execv("./cryption", args);
        std::cerr << "Error executing cryption" << std::endl;
        exit(1);
    } else if (childProcessToRun > 0) {
        int status;
        waitpid(childProcessToRun, &status, 0);
    } else {
        std::cerr << "Fork failed" << std::endl;
    }
#endif
}

void ProcessManagement::executeTasks() {
    // Now handled by worker threads automatically
}
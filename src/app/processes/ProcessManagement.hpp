// src/app/processes/ProcessManagement.hpp
#pragma once

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Task.hpp"

class ProcessManagement {
public:
    explicit ProcessManagement(size_t max_threads = 4);
    ~ProcessManagement();  // Declaration only
    
    ProcessManagement(const ProcessManagement&) = delete;
    ProcessManagement& operator=(const ProcessManagement&) = delete;
    
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
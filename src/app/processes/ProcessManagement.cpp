#include <iostream>
#include "ProcessManagement.hpp"
#include <windows.h>  // Windows-specific API
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include "../encryptDecrypt/Cryption.hpp"


//made compatible for windows

ProcessManagement::ProcessManagement() {}

bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task) {
    taskQueue.push(std::move(task));
    return true;
}

//Process Management

void ProcessManagement::executeTasks() {
    while (!taskQueue.empty()) {
        std::unique_ptr<Task> taskToExecute = std::move(taskQueue.front());
        taskQueue.pop();
        std::cout << "Executing task: " << taskToExecute->toString() << std::endl;
        executeCryption(taskToExecute->toString());

#ifdef _WIN32
        // Convert std::string to std::wstring (char → wchar_t)
        std::string taskStr = taskToExecute->toString();
        std::string command = "cryption.exe " + taskStr;  // Ensure cryption.exe exists

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
            exit(1);
        }

        // Wait for the child process to complete
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

#else
        int childProcessToRun = fork();
        if (childProcessToRun == 0) {
            std::string taskStr = taskToExecute->toString();
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
            exit(1);
        }
#endif
    }
}

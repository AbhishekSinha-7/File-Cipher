// Task.hpp
#ifndef TASK_HPP
#define TASK_HPP

#include "../fileHandling/IO.hpp"
#include <fstream>
#include <string>
#include <sstream>

enum class Action {
    ENCRYPT,
    DECRYPT
};

struct Task {
    std::string filePath;
    std::fstream f_stream;
    Action action;

    Task(std::fstream&& stream, Action act, std::string filePath) : f_stream(std::move(stream)), action(act), filePath(filePath) {}
    
    //serialise the task data
    std::string toString() const {

        std::ostringstream oss;

        // Serialise the file path and action
        oss << filePath << "," << (action == Action::ENCRYPT ? "ENCRYPT" : "DECRYPT");

        return oss.str();
    }

    //deserialise the task data
    static Task fromString(const std::string& taskData) {

        // Extract the file path and action from the task data
        std::istringstream iss(taskData);
        std::string filePath;
        std::string actionStr;

        // Check if the file path and action can be extracted
        if (std::getline(iss, filePath, ',') && std::getline(iss, actionStr)) {
            Action action = (actionStr == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;
            IO io(filePath);
            std::fstream f_stream = std::move(io.getFileStream());

            if (f_stream.is_open()) {

                // Return a new task object
                return Task(std::move(f_stream), action, filePath);
            } else {

                // Throw an exception if the file cannot be opened
                throw std::runtime_error("Failed to open file: " + filePath);
            }
        } else {

            // Throw an exception if the task data is invalid
            throw std::runtime_error("Invalid task data format");
        }
    }
};

#endif
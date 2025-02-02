#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"

int executeCryption(const std::string& taskData) {

    // Deserialise the task data
    Task task = Task::fromString(taskData);
    // Read the key from the environment file
    ReadEnv env;

    // Get the key from the environment file
    std::string envKey = env.getenv();
    int key = std::stoi(envKey);

    // Encrypt or decrypt the file

    if (task.action == Action::ENCRYPT) {
        char ch;

        // Iterate over each character in the file
        while (task.f_stream.get(ch)) {

            // Encryption
            ch = (ch + key) % 256;

            // Move the put pointer one position back
            task.f_stream.seekp(-1, std::ios::cur);

            // Overwrite the existing character
            task.f_stream.put(ch);
        }
        task.f_stream.close();
    } else {
        char ch;
        while (task.f_stream.get(ch)) {

            // Decryption
            ch = (ch - key + 256) % 256;

            // Move the put pointer one position back
            task.f_stream.seekp(-1, std::ios::cur);

            // Overwrite the existing character
            task.f_stream.put(ch);
        }
        task.f_stream.close();
    }
    return 0;
}
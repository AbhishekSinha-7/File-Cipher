#include "IO.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class ReadEnv{
    public:
        std::string getenv(){

            //creating a path to the .env file
            std::string env_path = ".env";

            //creating an IO object
            IO io(env_path);

            //Getting a File Stream
            std::fstream f_stream = io.getFileStream();

            //reading f_stream contents into a stringstream
            std::stringstream buffer;
            buffer << f_stream.rdbuf();

            //converting buffer to string
            std::string content = buffer.str();

            //returning the file content
            return content;
        }    
};
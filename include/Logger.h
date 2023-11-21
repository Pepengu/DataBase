#pragma once

#include <fstream>
#include <chrono>
#include <ctime>
#include <stdexcept>

namespace DB{
    class Logger {
        std::ofstream _file;
    public:
        void open(const char *file){
            _file.open(file);

            if(!_file.is_open()){
                throw std::runtime_error("Logger file can not be oppened");
            }
        }

        template<typename T>
        Logger &operator<< (const T &action){
            if(!_file.is_open()){
                throw std::runtime_error("Logger file is out of reach");
            }

            std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string timeString = std::ctime(&time);
            _file << timeString.substr(0,timeString.size()-1) << ": " << action << std::endl;

            return *this;
        }

        ~Logger(){
            _file.close();
        }
    };
}
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include "DataBase.h"

namespace DB{
    class DataBaseClient : DataBase{
    private:
        std::string _request;
        char _addr[17];
        char _buffer[1024];
        int _client_fd;

        size_t _processAddress(std::ifstream &cfg);

        /**Creates string that consists of 4 parts:
         * 1)useraname                  (string\0)
         * 2)password                   (string\0)
         * 3)structure of DB length     (8 byte number\0)
         * 4)structure of DB            (type:name\0)
        */
        void _create_request();
    public:
        DataBaseClient(const char* configFile);
        DataBaseClient(){}

        void init(const char* configFile);
        
        ~DataBaseClient(){close(_client_fd);}

        std::string send_command(const std::string &command);
    };
}
#include "DataBaseClient.h"

namespace DB{
    size_t DataBaseClient::_processAddress(std::ifstream &cfg){
        cfg.getline(_addr, 17);

        return 0;
    }

    void DataBaseClient::_create_request(){
        log << "Performing first connection";
        std::string size = std::to_string(_structure.size());
        size.push_back('\0');

        bool success = false;
        while(!success){
            std::string username, password;
            std::cout << "Enter username: ";
            std::cin >> username;
            username.push_back('\0');
            std::cout << "Enter password: ";
            std::cin >> password;
            password.push_back('\0');

            _request = username + password + size + _structureToString();
            std::string msg = _request + static_cast<char>(connection);

            char buffer[1024] = { 0 };
            int client_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (client_fd < 0) {
                perror("Socket creation error");
                exit(EXIT_FAILURE);
            }

            struct sockaddr_in servaddr;
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(PORT);
            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

            int status = connect(
                client_fd, 
                (struct sockaddr*)&servaddr,
                sizeof(servaddr)
            );
            if (status < 0) {
                perror("Connection Failed");
                exit(EXIT_FAILURE);
            }

            send(client_fd, msg.c_str(), msg.size(), 0);
            int valread = read(client_fd, buffer, 1024);
            
            if(valread > 0){
                if(strcmp(buffer, "Success") == 0){
                    success = true;
                }

                else if(strcmp(buffer, "Wrong structure") == 0){
                    throw std::runtime_error(buffer);
                }

                else{
                    if(strcmp(buffer, "Structures differ") == 0){
                        throw std::runtime_error("Structures of client and server are different");
                    }
                    std::cout << "Connection failed:" << buffer << std::endl;
                }
            }

            // closing the connected socket
            close(client_fd);
        }
    }
    
    DataBaseClient::DataBaseClient(const char* configFile): _addr(), _buffer(), _client_fd(socket(AF_INET, SOCK_STREAM, 0)){
        try{
            log.open("ClientLog.txt");
            _configMap["address"] = std::bind(&DataBaseClient::_processAddress, this, std::placeholders::_1);
            
            _parseConfig(configFile);

            _create_request();

            log << "Connection succesful";
            std::cout << "Connection succesful";
        }
        catch(...){
            throw;
        }
    }
}
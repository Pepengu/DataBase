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
            try{
                std::string res = send_command(std::to_string(static_cast<char>(connection)));
                if(res == "Success"){
                    success = true;
                }
            }
            catch(...){
                throw;
            }
        }
    }
    
    DataBaseClient::DataBaseClient(const char* configFile): _addr(), _buffer(), _client_fd(socket(AF_INET, SOCK_STREAM, 0)){
        init(configFile);
    }

    void DataBaseClient::init(const char* configFile){
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


    std::string DataBaseClient::send_command(const std::string &command){
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
        servaddr.sin_addr.s_addr = inet_addr(_addr);

        int status = connect(
            client_fd, 
            (struct sockaddr*)&servaddr,
            sizeof(servaddr)
        );
        if (status < 0) {
            perror("Connection Failed");
            exit(EXIT_FAILURE);
        }

        std::string msg = _request + command;
        send(client_fd, msg.c_str(), msg.size(), 0);
        int valread = read(client_fd, buffer, 1);
        
        std::string result;
        if(valread > 0){
            DB::STATUS status = DB::STATUS(buffer[0]);
            switch (status){
                case DB::STATUS::connection_success:
                default:
                    return result = ("Success");

                case DB::STATUS::username_invalid:
                case DB::STATUS::password_invalid:
                    result = std::string("Username or password is invalid");

                case DB::STATUS::structure_differ:
                    throw std::runtime_error("Structure differ");
                
            }
        }

        // closing the connected socket
        close(client_fd);

        return result;
    }
}
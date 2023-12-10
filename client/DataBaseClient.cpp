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
                std::string res = send_command(std::string{DB::REQUEST_TYPE::connection});
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
            std::cout << "Connection succesful\n";
        }
        catch(...){
            throw;
        }
    } 
}

std::string DB::DataBaseClient::send_command(const std::string &command){
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
    int valread = read(client_fd, buffer, 1024);

    std::string result;
    if(valread > 0){
        DB::STATUS status = DB::STATUS(buffer[0]);
        switch (status){
            default:
            case DB::STATUS::connection_success:
            case DB::STATUS::addition_success:
            case DB::STATUS::edition_success:
            case DB::STATUS::save_success:
            case DB::STATUS::stop_success:
            case DB::STATUS::removal_success:
                result = std::string("Success");
                break;

            case DB::STATUS::size_success:
            case DB::STATUS::get_success:
                result = std::string(buffer+1);
                break;

            case DB::STATUS::username_invalid:
            case DB::STATUS::password_invalid:
                result = std::string("Username or password is invalid");
                break;
            
            case DB::STATUS::not_enough_fields:
                result = std::string("Not enough fields in unnamed representation");
                break;

            case DB::STATUS::field_does_not_exist:
                result = std::string("Unknown field found");
                break;
            
            case DB::STATUS::record_does_not_exist:
                result.resize(0);
                break;
            
            case DB::STATUS::save_file_inaccessible:
                throw std::runtime_error("Save file inaccessible. Server crushed");
                break;

            case DB::STATUS::structure_differ:
                throw std::runtime_error("Structure differ");
            
        }
    }

    // closing the connected socket
    close(client_fd);

    return result;
}
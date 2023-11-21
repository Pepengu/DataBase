#include "DataBaseClient.h"

namespace DB{
    size_t DataBaseClient::_processAddress(std::ifstream &cfg){
        cfg.getline(_addr, 17);

        if (_client_fd < 0) {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr(_addr);

        int status = connect(
            _client_fd, 
            (struct sockaddr*)&servaddr,
            sizeof(servaddr)
        );
        if (status < 0) {
            perror("Connection Failed");
            exit(EXIT_FAILURE);
        }
        return 0;
    }

    void DataBaseClient::_create_request(){
        std::string username, password;
        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;

        char size[9]{0};
        size_t s = _structure.size();
        memcpy(size, reinterpret_cast<char*>(&s), 8);
        _request = username + "\0" + password + "\0" + size + "\0" + _structureToString();
    }
    
    DataBaseClient::DataBaseClient(const char* configFile): _addr(), _buffer(), _client_fd(socket(AF_INET, SOCK_STREAM, 0)){
        log.open("ClientLog.txt");
        _configMap["address"] = std::bind(&DataBaseClient::_processAddress, this, std::placeholders::_1);
        
        _parseConfig(configFile);

        _create_request();
    }
}
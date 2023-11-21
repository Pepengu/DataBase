#include "DataBaseServer.h"
#include <iostream>
#include <functional>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace DB{    
    size_t DataBaseServer::_processUsers(std::ifstream &cfg){
        bool isMultiline = cfg.peek() == '{';
        if(isMultiline){
            cfg.ignore();
        }

        bool endFlag = true;
        size_t line = (isMultiline ? 0 : 1);
        while (endFlag){
            while (cfg.peek() < 33){
                cfg.ignore();
            }

            std::string str;
            getline(cfg, str, '\n');

            std::string login = str.substr(0, str.find_first_of(':'));
            login = login.substr(0,login.find_first_of(' '));
            std::string password = str.substr(str.find_first_of(':')+1);
            password = password.substr(password.find_first_of(' ')); 

            _accounts[login] = password;

            endFlag = isMultiline && cfg.peek() != '}';
            line++;
        }
        return line;
    }

    size_t DataBaseServer::_processFile(std::ifstream &cfg){
        getline(cfg, _file, '\n');

        std::ifstream file(_file, std::ios_base::binary);
        if(!file.is_open()){
            std::ofstream temp(_file);
            temp.close();
            file.open(_file, std::ios_base::binary);
            if(!file.is_open()){
                throw std::invalid_argument("File can not be accesed");
            }
            return 0;
        }

        std::string fileType(DB::signature.size(), 0);
        for(size_t idx = 0; idx < DB::signature.size(); ++idx){
            file.get(fileType[idx]);
        }
        
        if(fileType != DB::signature){
            std::cout << fileType << std::endl;
            file.close();
            throw std::invalid_argument("DataBase file is not compatible or corrupted");
        }

        for(const auto &field : _structure){
            auto &ptr = *field.get();
            if(file.get() != DB::_hash2Idx[typeid(ptr).hash_code()]){
                file.close();
                throw std::invalid_argument("DataBase file has a different structure");
            }
        }


        while (file.peek() != EOF){
            std::vector<std::unique_ptr<DB::Field>> new_Entry(_structure.size()); 
            for(int field = 0; field < _structure.size(); ++field){
                char input[128];
                auto &ptr = *_structure[field].get();
                switch (DB::_hash2Idx[typeid(ptr).hash_code()]){
                case 0:
                    file.read(input, sizeof(int8_t));
                    new_Entry[field] = createField(*reinterpret_cast<int8_t*>(input));
                    break;
                case 1:
                    file.read(input, sizeof(int16_t));
                    new_Entry[field] = createField(*reinterpret_cast<int16_t*>(input));
                    break;
                case 2:
                    file.read(input, sizeof(int32_t));
                    new_Entry[field] = createField(*reinterpret_cast<int32_t*>(input));
                    break;
                case 3:
                    file.read(input, sizeof(int64_t));
                    new_Entry[field] = createField(*reinterpret_cast<int64_t*>(input));
                    break;
                case 4:
                    file.read(input, sizeof(uint8_t));
                    new_Entry[field] = createField(*reinterpret_cast<uint8_t*>(input));
                    break;
                case 5:
                    file.read(input, sizeof(uint16_t));
                    new_Entry[field] = createField(*reinterpret_cast<uint16_t*>(input));
                    break;
                case 6:
                    file.read(input, sizeof(uint32_t));
                    new_Entry[field] = createField(*reinterpret_cast<uint32_t*>(input));
                    break;
                case 7:
                    file.read(input, sizeof(uint64_t));
                    new_Entry[field] = createField(*reinterpret_cast<uint64_t*>(input));
                    break;
                case 8:
                    file.read(input, sizeof(bool));
                    new_Entry[field] = createField(*reinterpret_cast<bool*>(input));
                    break;
                case 9:
                    file.read(input, sizeof(double));
                    new_Entry[field] = createField(*reinterpret_cast<double*>(input));
                    break;
                case 10:
                    file.getline(input, 128, '\0');
                    new_Entry[field] = createField(input);
                    break;
                }
            }
            _entries.push_back(std::move(new_Entry));
        }

        return 0;
    }

    size_t DataBaseServer::_processBackupFrequency(std::ifstream &cfg){
        cfg >> _backup_frequency;
        return 0;
    }

    DataBaseServer::DataBaseServer(const char* configFile): _backup_count(100){
        log.open("ServerLog.txt");

        _configMap["users"] = std::bind(&DataBaseServer::_processUsers, this, std::placeholders::_1);
        _configMap["file"] = std::bind(&DataBaseServer::_processFile, this, std::placeholders::_1);
        _configMap["backup_frequency"] = std::bind(&DataBaseServer::_processBackupFrequency, this, std::placeholders::_1);

        _parseConfig(configFile);

        save();
    }

    void DB::DataBaseServer::addRecord(const Entry &entry){
        _entries.resize(_entries.size()+1);
        _entries.back().resize(_structure.size());
        for(size_t field = 0; field < _structure.size(); ++field){
            DB::Field::copyField(entry[field], _entries.back()[field]);
        }
    }

    inline Entry &DataBaseServer::operator[](size_t idx){
        if(idx >= _entries.size() || _entries[idx].size() == 0) {
            throw std::invalid_argument("Entry with id " + std::to_string(idx) + " does not exist");
        }
        return _entries[idx]; 
    }

    void DataBaseServer::remove(size_t idx){
        if(idx > _entries.size() || _entries[idx].size() == 0) {
            return;
        }
        _entries[idx].resize(0);
    }

    void DataBaseServer::save(){
        std::ofstream out(_file, std::ios_base::binary);
        if(!out.is_open()){
            throw "Save file can not be accesed. Make sure it is not being used by other program";
        }

        out << DB::signature;
        for(const auto &field : _structure){
            auto &ptr = *field.get();
            out.put(DB::_hash2Idx[typeid(ptr).hash_code()]);
        }

        for(const auto &entry : _entries){
            for(const auto &field : entry){
                out << field->getValue();
            }
        }
    }   
}
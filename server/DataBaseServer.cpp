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
            std::string password = str.substr(str.find_first_of(':') + 1);
            size_t leading_spaces = password.find_first_of(' ');
            if(leading_spaces != std::string::npos){
                password = password.substr(password.find_first_of(' ') + 1); 
            }

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
            if(file.get() != DB::_hash2Type[typeid(ptr).hash_code()]){
                file.close();
                throw std::invalid_argument("DataBase file has a different structure");
            }
        }


        while (file.peek() != EOF){
            std::vector<std::unique_ptr<DB::Field>> new_Entry(_structure.size()); 
            for(int field = 0; field < _structure.size(); ++field){
                char input[128];
                auto &ptr = *_structure[field].get();
                switch (DB::_hash2Type[typeid(ptr).hash_code()]){
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


    bool DataBaseServer::_filterEntry(char* entry){
        bool is_named = false;
        for(int i = 0; entry[i] != 0; ++i){
            if(entry[i] == ':'){
                is_named = true;
                entry[i] = 0;
            }
        }
        return is_named;
    }

    Entry DataBaseServer::_createNamed(const char* str){
        DB::Entry entry(_structure.size());
        for(size_t i = 0; i < _structure.size(); ++i){
            DB::Field::copyField(_structure[i], entry[i]);
        }

        size_t idx = 0;
        while(str[idx] != 0){
            std::string name(str+idx);
            for(;str[idx++]!=0;);

            entry[_name2idx[name]]->setValue(str+(++idx));
            for(;str[idx++]!=0;);
            ++idx;            
        }

        return std::move(entry);
    }

    Entry DataBaseServer::_createUnnamed(const char* str){
        DB::Entry entry(_structure.size());
        for(size_t i = 0; i < _structure.size(); ++i){
            if(*str == 0){
                throw not_enough_fields;
            }
            DB::Field::copyField(_structure[i], entry[i]);
            entry[i]->setValue(str);
            for(;str[i]!=' ' && str[i]!=0;++i);
        }
        return std::move(entry);
    }

    void DataBaseServer::_addNamed(const char* str){
        _entries.push_back(std::move(_createNamed(str)));
    }

    void DataBaseServer::_addUnamed(const char* str){

        _entries.push_back(std::move(_createUnnamed(str)));
    }

    void DataBaseServer::_editNamed(size_t idx, const char* str){
        size_t i = 0;
        while(str[i] != 0){
            std::string name(str+idx);
            for(;str[i++]!=0;);

            _entries[idx][_name2idx[name]]->setValue(str+(++idx));
            for(;str[idx++]!=0;);
            ++idx;            
        }
    }

    void DataBaseServer::_editUnamed(size_t idx, const char* str){
        _entries[idx] = std::move(_createUnnamed(str));
    }

    DataBaseServer::DataBaseServer(const char* configFile): _backup_count(100){
        init(configFile);
    }

    void DataBaseServer::init(const char* configFile){
        log.open("ServerLog.txt");

        _configMap["users"] = std::bind(&DataBaseServer::_processUsers, this, std::placeholders::_1);
        _configMap["file"] = std::bind(&DataBaseServer::_processFile, this, std::placeholders::_1);
        _configMap["backup_frequency"] = std::bind(&DataBaseServer::_processBackupFrequency, this, std::placeholders::_1);

        _parseConfig(configFile);

        save();
    }

    void DataBaseServer::close(){
        try{
            save();
        }
        catch(...){
            throw;
        }
    }

    size_t DB::DataBaseServer::validateRequest(const char* request){
        std::string structure = _structureToString();
        std::string username(request);
        size_t request_idx = strlen(request) + 1;
        std::string password(request + request_idx);
        request_idx += strlen(request + request_idx) + 1;

        if(_accounts.find(username) == _accounts.end()){
            throw DB::STATUS::username_invalid;
        }

        if(_accounts[username] != password){
            std::cout << (int)_accounts[username][0] << std::endl;
            std::cout << password << "" << _accounts[username] << std::endl;
            throw DB::STATUS::password_invalid;
        }

        if(_structure.size() != std::atoi(request + request_idx)){
            throw DB::STATUS::structure_differ;
        }
        request_idx += strlen(request + request_idx) + 1;

        for(int structure_idx = 0; 
                request_idx < 1024 && structure_idx < _structure.size();
                ++structure_idx, ++request_idx){
            if(request[request_idx] != structure[structure_idx]){
                throw DB::STATUS::structure_differ;
            }
        }

        return request_idx+1;
    }

    void DB::DataBaseServer::addRecord(const Entry &entry){
        _entries.resize(_entries.size()+1);
        _entries.back().resize(_structure.size());
        for(size_t field = 0; field < _structure.size(); ++field){
            DB::Field::copyField(entry[field], _entries.back()[field]);
        }
    }

    void DB::DataBaseServer::addRecord(Entry &&entry){
        _entries.resize(_entries.size()+1);
        _entries.back() = std::move(entry);
    }

    void DB::DataBaseServer::addRecord(char* entry){
        bool is_named = _filterEntry(entry);
        try{
            if(is_named){
                _addNamed(entry);
            }
            else{
                _addUnamed(entry);
            }
        }
        catch(...){
            throw;
        }
    }

    void DB::DataBaseServer::editRecord(char* entry){
        size_t idx = *reinterpret_cast<size_t*>(entry);
        if(idx > _entries.size() || _entries[idx].size() == 0){
            throw DB::STATUS::field_does_not_exist;
        }
        entry += sizeof(size_t);
        bool is_named = _filterEntry(entry);
        try{
            if(is_named){
                _editNamed(idx, entry);
            }
            else{
                _editUnamed(idx, entry);
            }
        }
        catch(...){
            throw;
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
        _entries[idx].clear();
    }

    void DataBaseServer::save(){
        std::ofstream out(_file, std::ios_base::binary);
        if(!out.is_open()){
            throw "Save file can not be accesed. Make sure it is not being used by other program";
        }

        out << DB::signature;
        for(const auto &field : _structure){
            auto &ptr = *field.get();
            out.put(DB::_hash2Type[typeid(ptr).hash_code()]);
        }

        for(const auto &entry : _entries){
            for(const auto &field : entry){
                out << field->getValue();
            }
        }
    }   

}
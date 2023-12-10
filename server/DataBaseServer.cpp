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
            log << "DataBase file is not compatible or corrupted";
            file.close();
            throw std::invalid_argument("DataBase file is not compatible or corrupted");
        }

        for(const auto &field : _structure){
            auto &ptr = *field.get();
            if(file.get() != DB::_hash2Type[typeid(ptr).hash_code()]){
                file.close();
                log << "DataBase file has a different structure";
                throw std::invalid_argument("DataBase file has a different structure");
            }
        }


        while (file.peek() != EOF){
            std::vector<std::unique_ptr<DB::Field>> new_Entry(_structure.size()); 
            for(size_t field = 0; field < _structure.size(); ++field){
                char input[128]{0};
                file.getline(input, 128, '\0');
                std::string val(input); 
                auto &ptr = *_structure[field].get();
                switch (DB::_hash2Type[typeid(ptr).hash_code()]){
                case 0:
                    new_Entry[field] = createField(static_cast<int8_t>(stol(val)));
                    break;
                case 1:
                    new_Entry[field] = createField(static_cast<int16_t>(stol(val)));
                    break;
                case 2:
                    new_Entry[field] = createField(stol(val));
                    break;
                case 3:
                    new_Entry[field] = createField(stoll(val));
                    break;
                case 4:
                    new_Entry[field] = createField(static_cast<uint8_t>(stoul(val)));
                    break;
                case 5:
                    new_Entry[field] = createField(static_cast<uint16_t>(stoul(val)));
                    break;
                case 6:
                    new_Entry[field] = createField(stoul(val));
                    break;
                case 7:
                    new_Entry[field] = createField(stoull(val));
                    break;
                case 8:
                    new_Entry[field] = createField(static_cast<bool>(stoi(val)));
                    break;
                case 9:
                    new_Entry[field] = createField(stod(val));
                    break;
                case 10:
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

    void DataBaseServer::_editNamed(size_t idx, const char* str){
        while(*str != 0){
            for(;isspace(*str); ++str);
            std::string name(str);
            str += name.size() + 1;
            for(;isspace(*str); ++str);
            size_t comma = 0;
            for(;str[comma] != ',' && str[comma] != 0; ++comma);
            std::string val(str, comma);
            _entries[idx][_name2idx[name]]->setValue(val.c_str());
            for(;!isspace(*str) && *str!=0;++str);   
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
            throw DB::STATUS::password_invalid;
        }

        if(_structure.size() != std::atoi(request + request_idx)){
            throw DB::STATUS::structure_differ;
        }
        request_idx += strlen(request + request_idx) + 1;

        for(size_t structure_idx = 0; 
                request_idx < 1024 && structure_idx < structure.size();
                ++structure_idx, ++request_idx){
            if(request[request_idx] != structure[structure_idx]){
                throw DB::STATUS::structure_differ;
            }
        }

        return request_idx;
    }

    void DB::DataBaseServer::addRecord(const Entry &entry){
        _entries.resize(_entries.size()+1);
        _entries.back().resize(_structure.size());
        for(size_t field = 0; field < _structure.size(); ++field){
            DB::Field::copyField(entry[field], _entries.back()[field]);
        }
    }

    void DB::DataBaseServer::addRecord(Entry &&entry){
        _entries.push_back(std::move(entry));
    }

    void DB::DataBaseServer::addRecord(char* entry){
        _entries.push_back(std::move(StringToEntry(entry)));
    }

    void DB::DataBaseServer::editRecord(char* entry){
        int space = 0;
        while(entry[space] != ' '){
            ++space;
        }
        entry[space] = 0;
        size_t idx = atoi(entry);
        if(idx >= _entries.size() || _entries[idx].size() == 0 || _entries[idx].size() == 0){
            throw DB::STATUS::record_does_not_exist;
        }
        entry += space+1;
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


    std::string DataBaseServer::get(size_t idx){
        if(idx < 0 || idx >= _entries.size() || _entries[idx].size() == 0){
            throw DB::STATUS::record_does_not_exist;
        }

        if(_entries[idx].size() == 0){
            return std::string("");
        }
        return getReadableEntry(_entries[idx]);
    }

    void DataBaseServer::remove(size_t idx){
        if(idx < 0 || idx >= _entries.size() || _entries[idx].size() == 0) {
            return;
        }

        if(idx + 1 == _entries.size()){
            _entries.pop_back();
        }
        else{
            _entries[idx].clear();
        }
    }

    void DataBaseServer::save(){
        std::ofstream out(_file, std::ios_base::binary);
        if(!out.is_open()){
            throw "Save file can not be accesed. Make sure it is not being used by other program";
        }

        out << DB::signature;
        for(const auto &field : _structure){
            auto &ptr = *field.get();
            out.put(static_cast<char>(DB::_hash2Type[typeid(ptr).hash_code()]));
        }

        for(const auto &entry : _entries){
            for(const auto &field : entry){
                out << field->getValue();
                out.put(0);
            }
        }
    }   

}
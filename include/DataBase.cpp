#include "DataBase.h"
#include <functional>
#include <iostream>

void DB::DataBase::_parseConfig(std::ifstream &cfg){
    size_t line = 1;
    while(cfg.peek() != EOF){
        if(cfg.peek() == '#' || cfg.peek() == '}' || cfg.peek() < 33){
            cfg.ignore(1024, '\n'); 
            ++line;
            continue;
        }

        std::string command;
        getline(cfg, command, '=');

        command = command.substr(command.find_first_not_of(' '));
        command = command.substr(0, command.find_first_of(' '));
        cfg.ignore();

        if(_configMap.find(command) == _configMap.end()){
            throw std::invalid_argument("Invalid confiuration command found in line " + std::to_string(line));
        }
        try{
            while (cfg.peek() < 33){
                cfg.ignore();
            }
            
            std::cout << "processing " << command << std::endl;
            line += _configMap[command](cfg);
        }
        catch(size_t l){
            throw std::invalid_argument("Wrong data type found in line " + std::to_string(line+l));
        }
        catch(...){
            throw;
        }

        ++line;
    }
}

size_t DB::DataBase::_processEntry(std::ifstream &cfg){
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

        std::string name = str.substr(0, str.find_first_of(':'));
        name = name.substr(0,name.find_last_not_of(' ')+1);
        std::string type = str.substr(str.find_first_of(':')+1, str.find_first_of(';')); 
        type = type.substr(type.find_first_not_of(' '));

        if(_str2Idx.find(type) == _str2Idx.end()){
            throw line;
        }
        _name2idx[name] = _structure.size();
        switch (_str2Idx[type]){
        case 0:
            _structure.push_back(createField(static_cast<int8_t>(DEFAULT_INT)));
            break;
        case 1:
            _structure.push_back(createField(static_cast<int16_t>(DEFAULT_INT)));
            break;
        case 2:
            _structure.push_back(createField(static_cast<int32_t>(DEFAULT_INT)));
            break;
        case 3:
            _structure.push_back(createField(static_cast<int64_t>(DEFAULT_INT)));
            break;
        case 4:
            _structure.push_back(createField(static_cast<uint8_t>(DEFAULT_INT)));
            break;
        case 5:
            _structure.push_back(createField(static_cast<uint16_t>(DEFAULT_INT)));
            break;
        case 6:
            _structure.push_back(createField(static_cast<uint32_t>(DEFAULT_INT)));
            break;
        case 7:
            _structure.push_back(createField(static_cast<uint64_t>(DEFAULT_INT)));
            break;
        case 8:
            _structure.push_back(createField(DEFAULT_BOOl));
            break;
        case 9:
            _structure.push_back(createField(DEFAULT_DOUBLE));
            break;
        case 10:
            _structure.push_back(createField(DEFAULT_STRING));
            break;
        
        default:
            throw line;
        }

        endFlag = isMultiline && cfg.peek() != '}';
        line++;
    }
    return line;
}

size_t DB::DataBase::_processAddress(std::ifstream &cfg){
    std::string addr;
    getline(cfg, addr, '\n');

    //TODO: network code

    return 0;
}
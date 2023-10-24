#include "DataBase.h"
#include <iostream>
#include <functional>

size_t DB::structure::nameToIdx(const std::string &str){
    return _name2idx[str];
}

size_t DB::structure::nameToIdx(const char* str){
    return nameToIdx(std::string(str));
}


void DB::structure::copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to){
    if(from.get() != nullptr){
        const DB::Field *ptr = from.get();
        switch (DB::_hash2Idx[typeid(*ptr).hash_code()]){
            case 0:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<int8_t>(*dynamic_cast<const DB::NumberField<int8_t>*>(ptr)));
                break;
            case 1:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<int16_t>(*dynamic_cast<const DB::NumberField<int16_t>*>(ptr)));
                break;
            case 2:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<int32_t>(*dynamic_cast<const DB::NumberField<int32_t>*>(ptr)));
                break;
            case 3:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<int64_t>(*dynamic_cast<const DB::NumberField<int64_t>*>(ptr)));
                break;
            case 4:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<uint8_t>(*dynamic_cast<const DB::NumberField<uint8_t>*>(ptr)));
                break;
            case 5:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<uint16_t>(*dynamic_cast<const DB::NumberField<uint16_t>*>(ptr)));
                break;
            case 6:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<uint32_t>(*dynamic_cast<const DB::NumberField<uint32_t>*>(ptr)));
                break;
            case 7:
                to = std::unique_ptr<DB::Field>(new DB::NumberField<uint64_t>(*dynamic_cast<const DB::NumberField<uint64_t>*>(ptr)));
                break;
            case 8:
                to = std::unique_ptr<DB::Field>(new DB::BoolField(*dynamic_cast<const DB::BoolField*>(ptr)));
                break;
            case 9:
                to = std::unique_ptr<DB::Field>(new DB::DoubleField(*dynamic_cast<const DB::DoubleField*>(ptr)));
                break;
            case 10:
                to = std::unique_ptr<DB::Field>(new DB::StringField(*dynamic_cast<const DB::StringField*>(ptr)));
                break;
            default:
                throw std::invalid_argument("Unknown field type occured");
                break;
        }
    }
    else{
        throw std::invalid_argument("Empty field occured");
    }
}

void DB::DataBase::parseConfig(std::ifstream &cfg){
    std::map<std::string, std::function<size_t(std::ifstream &cfg)>> m{
        {"entry", std::bind(&DB::DataBase::processEntry, this, std::placeholders::_1)},
        {"users", std::bind(&DB::DataBase::processUsers, this, std::placeholders::_1)},
        {"address", std::bind(&DB::DataBase::processAddress, this, std::placeholders::_1)},
        {"file", std::bind(&DB::DataBase::processFile, this, std::placeholders::_1)},
        {"backup_frequency", std::bind(&DB::DataBase::processBackupFrequency, this, std::placeholders::_1)}
    };

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

        if(m.find(command) == m.end()){
            throw std::invalid_argument("Invalid confiuration command found in line " + std::to_string(line));
        }
        try{
            while (cfg.peek() < 33){
                cfg.ignore();
            }
            
            line += m[command](cfg);
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

size_t DB::DataBase::processEntry(std::ifstream &cfg){
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
        switch (_str2Idx[type]){
        case 0:
            _struct._entry.push_back(std::make_unique<DB::NumberField<int8_t>>(name));
            break;
        case 1:
            _struct._entry.push_back(std::make_unique<DB::NumberField<int16_t>>(name));
            break;
        case 2:
            _struct._entry.push_back(std::make_unique<DB::NumberField<int32_t>>(name));
            break;
        case 3:
            _struct._entry.push_back(std::make_unique<DB::NumberField<int64_t>>(name));
            break;
        case 4:
            _struct._entry.push_back(std::make_unique<DB::NumberField<uint8_t>>(name));
            break;
        case 5:
            _struct._entry.push_back(std::make_unique<DB::NumberField<uint16_t>>(name));
            break;
        case 6:
            _struct._entry.push_back(std::make_unique<DB::NumberField<uint32_t>>(name));
            break;
        case 7:
            _struct._entry.push_back(std::make_unique<DB::NumberField<uint64_t>>(name));
            break;
        case 8:
            _struct._entry.push_back(std::make_unique<DB::BoolField>(name));
            break;
        case 9:
            _struct._entry.push_back(std::make_unique<DB::DoubleField>(name));
            break;
        case 10:
            _struct._entry.push_back(std::make_unique<DB::StringField>(name));
            break;
        
        default:
            throw line;
        }

        endFlag = isMultiline && cfg.peek() != '}';
        line++;
    }
    return line;
}

size_t DB::DataBase::processUsers(std::ifstream &cfg){
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

size_t DB::DataBase::processAddress(std::ifstream &cfg){
    std::string addr;
    getline(cfg, addr, '\n');

    //TODO: network code

    return 0;
}

size_t DB::DataBase::processFile(std::ifstream &cfg){
    std::string fileName;
    getline(cfg, fileName, '\n');

    std::ifstream file(fileName, std::ios_base::binary);
    if(!file.is_open()){
        std::ofstream temp(fileName);
        temp.close();
        file.open(fileName, std::ios_base::binary);
    }
    if(!file.is_open()){
        throw std::invalid_argument("File can not be accesed");
    }

    //TODO: loading database from file

    return 0;
}

size_t DB::DataBase::processBackupFrequency(std::ifstream &cfg){
    cfg >> _backup_frequency;
    return 0;
}
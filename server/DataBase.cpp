#include "DataBase.h"
#include <functional>

void DB::DataBase::_parseConfig(std::ifstream &cfg){
    std::map<std::string, std::function<size_t(std::ifstream &cfg)>> m{
        {"entry", std::bind(&DB::DataBase::_processEntry, this, std::placeholders::_1)},
        {"users", std::bind(&DB::DataBase::_processUsers, this, std::placeholders::_1)},
        {"address", std::bind(&DB::DataBase::_processAddress, this, std::placeholders::_1)},
        {"file", std::bind(&DB::DataBase::_processFile, this, std::placeholders::_1)},
        {"backup_frequency", std::bind(&DB::DataBase::_processBackupFrequency, this, std::placeholders::_1)}
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
            
            std::cout << "processing " << command << std::endl;
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
            _structure.push_back(std::make_unique<DB::NumberField<int8_t>>());
            break;
        case 1:
            _structure.push_back(std::make_unique<DB::NumberField<int16_t>>());
            break;
        case 2:
            _structure.push_back(std::make_unique<DB::NumberField<int32_t>>());
            break;
        case 3:
            _structure.push_back(std::make_unique<DB::NumberField<int64_t>>());
            break;
        case 4:
            _structure.push_back(std::make_unique<DB::NumberField<uint8_t>>());
            break;
        case 5:
            _structure.push_back(std::make_unique<DB::NumberField<uint16_t>>());
            break;
        case 6:
            _structure.push_back(std::make_unique<DB::NumberField<uint32_t>>());
            break;
        case 7:
            _structure.push_back(std::make_unique<DB::NumberField<uint64_t>>());
            break;
        case 8:
            _structure.push_back(std::make_unique<DB::BoolField>());
            break;
        case 9:
            _structure.push_back(std::make_unique<DB::DoubleField>());
            break;
        case 10:
            _structure.push_back(std::make_unique<DB::StringField>());
            break;
        
        default:
            throw line;
        }

        endFlag = isMultiline && cfg.peek() != '}';
        line++;
    }
    return line;
}

size_t DB::DataBase::_processUsers(std::ifstream &cfg){
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

size_t DB::DataBase::_processAddress(std::ifstream &cfg){
    std::string addr;
    getline(cfg, addr, '\n');

    //TODO: network code

    return 0;
}

size_t DB::DataBase::_processFile(std::ifstream &cfg){
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
        file.close();
        throw std::invalid_argument("Database file is not compatible or corrupted");
    }

    for(const auto &field : _structure){
        auto &ptr = *field.get();
        if(file.get() != DB::_hash2Idx[typeid(ptr).hash_code()]){
            file.close();
            throw std::invalid_argument("Database file has a different structure");
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
                new_Entry[field] = std::make_unique<NumberField<int8_t>>(*reinterpret_cast<int8_t*>(input));
                break;
            case 1:
                file.read(input, sizeof(int16_t));
                new_Entry[field] = std::make_unique<NumberField<int16_t>>(*reinterpret_cast<int16_t*>(input));
                break;
            case 2:
                file.read(input, sizeof(int32_t));
                new_Entry[field] = std::make_unique<NumberField<int32_t>>(*reinterpret_cast<int32_t*>(input));
                break;
            case 3:
                file.read(input, sizeof(int64_t));
                new_Entry[field] = std::make_unique<NumberField<int64_t>>(*reinterpret_cast<int64_t*>(input));
                break;
            case 4:
                file.read(input, sizeof(uint8_t));
                new_Entry[field] = std::make_unique<NumberField<uint8_t>>(*reinterpret_cast<uint8_t*>(input));
                break;
            case 5:
                file.read(input, sizeof(uint16_t));
                new_Entry[field] = std::make_unique<NumberField<uint16_t>>(*reinterpret_cast<uint16_t*>(input));
                break;
            case 6:
                file.read(input, sizeof(uint32_t));
                new_Entry[field] = std::make_unique<NumberField<uint32_t>>(*reinterpret_cast<uint32_t*>(input));
                break;
            case 7:
                file.read(input, sizeof(uint64_t));
                new_Entry[field] = std::make_unique<NumberField<uint64_t>>(*reinterpret_cast<uint64_t*>(input));
                break;
            case 8:
                file.read(input, sizeof(bool));
                new_Entry[field] = std::make_unique<BoolField>(*reinterpret_cast<bool*>(input));
                break;
            case 9:
                file.read(input, sizeof(double));
                new_Entry[field] = std::make_unique<DoubleField>(*reinterpret_cast<double*>(input));
                break;
            case 10:
                file.getline(input, 128, '\0');
                new_Entry[field] = std::make_unique<StringField>(input);
                break;
            }
        }
        _entries.push_back(std::move(new_Entry));
    }

    return 0;
}

size_t DB::DataBase::_processBackupFrequency(std::ifstream &cfg){
    cfg >> _backup_frequency;
    return 0;
}

void DB::DataBase::addRecord(const Entry &entry){
    _entries.resize(_entries.size()+1);
    _entries.back().resize(_structure.size());
    for(size_t field = 0; field < _structure.size(); ++field){
        DB::Field::copyField(entry[field], _entries.back()[field]);
    }
}

void DB::DataBase::remove(size_t idx){
    if(idx > _entries.size() || _entries[idx].size() == 0) {
        return;
    }
}

void DB::DataBase::save(){
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
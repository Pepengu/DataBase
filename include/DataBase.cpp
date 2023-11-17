#include "DataBase.h"
#include <functional>
#include <iostream>

namespace DB{
// ===========================================================================
// ============================= Entry functions =============================
// ===========================================================================

    void Entry::swap(Entry &&other){
        std::swap(_entry, other._entry);
    }

    Entry::Entry(const std::vector<std::unique_ptr<DB::Field>> &v): _entry(v.size()){
        for(size_t i = 0; i < _entry.size(); ++i){
            Field::copyField(v[i], _entry[i]);
        }
    }

    Entry::Entry(std::vector<std::unique_ptr<DB::Field>> &&v): _entry(v.size()){
        for(size_t i = 0; i < _entry.size(); ++i){
            _entry[i] = std::move(v[i]);
        }
    }

    Entry::Entry(const Entry &other): _entry(other._entry.size()){
        for(size_t field = 0; field < _entry.size(); ++field){
            Field::copyField(other._entry[field], _entry[field]);
        }
    }

    Entry::Entry(Entry &&other){
        swap(std::move(other));
    }

    Entry &Entry::operator=(const Entry &other){
        _entry.resize(other._entry.size());
        for(size_t i = 0; i < other._entry.size(); ++i){
            Field::copyField(other._entry[i], _entry[i]);
        }

        return *this;
    }

    Entry &Entry::operator=(Entry &&other){
        swap(std::move(other));
        return *this;
    }

    std::unique_ptr<DB::Field> &Entry::operator [](const size_t idx){
        return _entry[idx];
    }

    const std::unique_ptr<DB::Field> &Entry::operator [](const size_t idx) const{
        return _entry[idx];
    }

    inline size_t Entry::size() const{
        return _entry.size();
    }

    inline void Entry::push_back(const std::unique_ptr<DB::Field> &field){
        _entry.resize(_entry.size()+1);
        Field::copyField(field, _entry.back());
    }

    inline void Entry::resize(size_t size){
        _entry.resize(size);
    }

// ==============================================================================
// ============================= DataBase functions =============================
// ==============================================================================
    std::string DataBase::structureToString(){
        std::string res;
        
        for(size_t i = 0; i < _structure.size(); ++i){
            std::string name("\0");
            for(const auto &[n, idx] : _name2idx){
                if(idx == i){
                    name = n;
                }
            }
            if(name == "\0"){
                throw std::runtime_error("structure is corrupted");
            }

            res += name + _structure[i]->getValue();
        }

        return res;
    }

    void DataBase::_parseConfig(std::ifstream &cfg){
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

    size_t DataBase::_processEntry(std::ifstream &cfg){
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
}

#include "DataBase.h"
#include <functional>
#include <iostream>

namespace DB{
    // ===========================================================================
    // ============================= Entry functions =============================
    // ===========================================================================
    std::vector<std::string> getValues(const Entry &entry){
        std::vector<std::string> values(entry.size()); 
        for(size_t idx = 0; idx < entry.size(); ++idx){
            values[idx] = entry[idx]->getValue();
        }
        return values;
    }

    std::vector<FIELDS> getTypes(const Entry &entry){
        std::vector<FIELDS> types(entry.size()); 
        for(size_t idx = 0; idx < entry.size(); ++idx){
            auto &ptr = *entry[idx].get();
            types[idx] = _hash2Idx[typeid(ptr).hash_code()];
        }
        return types;
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

    void DataBase::_parseConfig(const char* configFile){
        std::ifstream cfg(configFile);
        
        if(!cfg.is_open()){
            throw std::invalid_argument("Config file can not be oppened");
        }
        
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

        cfg.close();
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
            case BYTE:
                _structure.push_back(createField(static_cast<int8_t>(DEFAULT_BYTE)));
                break;
            case SHORT:
                _structure.push_back(createField(static_cast<int16_t>(DEFAULT_SHORT)));
                break;
            case INT:
                _structure.push_back(createField(static_cast<int32_t>(DEFAULT_INT)));
                break;
            case LONG:
                _structure.push_back(createField(static_cast<int64_t>(DEFAULT_LONG)));
                break;
            case UBYTE:
                _structure.push_back(createField(static_cast<uint8_t>(DEFAULT_UBYTE)));
                break;
            case USHORT:
                _structure.push_back(createField(static_cast<uint16_t>(DEFAULT_USHORT)));
                break;
            case UINT:
                _structure.push_back(createField(static_cast<uint32_t>(DEFAULT_UINT)));
                break;
            case ULONG:
                _structure.push_back(createField(static_cast<uint64_t>(DEFAULT_ULONG)));
                break;
            case BOOL:
                _structure.push_back(createField(DEFAULT_BOOl));
                break;
            case DOUBLE:
                _structure.push_back(createField(DEFAULT_DOUBLE));
                break;
            case STRING:
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

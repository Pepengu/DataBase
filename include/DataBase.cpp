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
            types[idx] = _hash2Type[typeid(ptr).hash_code()];
        }
        return types;
    }

// ==============================================================================
// ============================= DataBase functions =============================
// ==============================================================================

    std::string DataBase::_structureToString(){
        std::string structure;
        auto types = getTypes(_structure);
        for(size_t idx = 0; idx < types.size(); ++idx){
            structure += std::string{static_cast<char>(types[idx])} + ":" + _idx2name[idx];
            structure.push_back('\0');
        }
        return structure;
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
                
                log << ("processing " + command);
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

            if(_str2Type.find(type) == _str2Type.end()){
                throw line;
            }

            _idx2name.push_back(name);
            _name2idx[name] = _structure.size();
            switch (_str2Type[type]){
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

    bool isNumber(std::string::const_iterator begin, std::string::const_iterator end){
        while(begin != end && *begin >= '0' && *begin <= '9'){
            ++begin;
        }

        return begin == end;
    }

    bool DataBase::_checkNamed(const std::string &entry){
        for(auto shift = entry.cbegin(); shift != entry.end();){
            while(shift != entry.end() && (std::isspace(*shift) || *shift == ',')){
                ++shift;
            }
            if(shift == entry.end()){
                break;
            }

            auto end = std::find(shift, entry.end(), ':');
            if(end == entry.end()){
                return false;
            }

            std::string name(shift, end);
            if(_name2idx.find(name) == _name2idx.end()){
                return false;
            }
            shift = end+1;
            while(shift != entry.end() && *shift == ' '){
                ++shift;
            }
            if(shift == entry.end()){
                return false;
            }

            auto &ptr = *_structure[_name2idx[name]].get();
            size_t hash = typeid(ptr).hash_code();
            std::string cur;
            switch (_hash2Type[hash]){
            case DB::FIELDS::BYTE:
            case DB::FIELDS::SHORT:
            case DB::FIELDS::INT:
            case DB::FIELDS::LONG:
                if(*shift == '-'){
                    ++shift;
                }

            case DB::FIELDS::UBYTE:
            case DB::FIELDS::USHORT:
            case DB::FIELDS::UINT:
            case DB::FIELDS::ULONG:
                end = std::find(shift, entry.end(), ',');
                
                if(!isNumber(shift, end)){
                    return false;
                }
                break;

            case DB::FIELDS::BOOL:
                end = std::find(shift, entry.end(), ',');
                cur = std::string(shift, end);
                if(cur != "TRUE" && cur != "FALSE"){
                    return false;
                }
                break;

            case DB::FIELDS::DOUBLE:
                end = std::find(shift, entry.end(), '.');
                if(!isNumber(shift, end)){
                    return false;
                }
                shift = end+1;
                end = std::find(shift, entry.end(), ',');
                if(!isNumber(shift, end)){
                    return false;
                }
                shift = end;
                break;

            case DB::FIELDS::STRING:
                end = std::find(shift, entry.end(), ',');
                break;
            }
            shift = end;
        }

        return true;
    }

    bool DataBase::_checkUnnamed(const std::string &entry){
        auto shift = entry.cbegin();
        for(size_t entry_size = 0; entry_size < _structure.size(); ++entry_size){
            while(shift != entry.end() && (std::isspace(*shift) || *shift == ',')){
                ++shift;
            }
            if(shift == entry.end()){
                return false;
            }

            std::string::const_iterator end;

            auto &ptr = *_structure[entry_size].get();
            size_t hash = typeid(ptr).hash_code();
            std::string cur;
            switch (_hash2Type[hash]){
            case DB::FIELDS::BYTE:
            case DB::FIELDS::SHORT:
            case DB::FIELDS::INT:
            case DB::FIELDS::LONG:
                if(*shift == '-'){
                    ++shift;
                }

            case DB::FIELDS::UBYTE:
            case DB::FIELDS::USHORT:
            case DB::FIELDS::UINT:
            case DB::FIELDS::ULONG:
                end = std::find(shift, entry.end(), ',');
                
                if(!isNumber(shift, end)){
                    return false;
                }
                break;

            case DB::FIELDS::BOOL:
                end = std::find(shift, entry.end(), ',');
                cur = std::string(shift, end);
                if(cur != "TRUE" && cur != "FALSE"){
                    return false;
                }
                break;

            case DB::FIELDS::DOUBLE:
                end = std::find(shift, entry.end(), '.');
                if(!isNumber(shift, end)){
                    return false;
                }
                shift = end+1;
                end = std::find(shift, entry.end(), ',');
                if(!isNumber(shift, end)){
                    return false;
                }
                break;

            case DB::FIELDS::STRING:
                end = std::find(shift, entry.end(), ',');
                break;
            }        
            shift = end;
        }
        return shift == entry.end();
    }

    bool DataBase::_filterEntry(char* entry){
        bool is_named = false;
        for(int i = 0; entry[i] != 0; ++i){
            if(entry[i] == ':'){
                is_named = true;
                entry[i] = 0;
            }
        }
        return is_named;
    }

    Entry DataBase::_createNamed(const char* str, bool use_default){
        DB::Entry entry(_structure.size());
        if(use_default){
            for(size_t i = 0; i < _structure.size(); ++i){
                DB::Field::copyField(_structure[i], entry[i]);
            }
        }      

        while(*str != 0){
            for(;isspace(*str); ++str);
            std::string name(str);
            str += name.size() + 1;
            for(;isspace(*str); ++str);

            if(!use_default){
                DB::Field::copyField(_structure[_name2idx[name]], entry[_name2idx[name]]);
            }

            size_t comma = 0;
            for(;str[comma] != ',' && str[comma] != 0; ++comma);
            std::string val(str, comma);
            entry[_name2idx[name]]->setValue(val.c_str());
            for(;!isspace(*str) && *str!=0;++str);   
        }

        return entry;
    }

    Entry DataBase::_createUnnamed(const char* str){
        DB::Entry entry(_structure.size());
        for(size_t i = 0; i < entry.size(); ++i){
            if(*str == 0){
                throw not_enough_fields;
            }
            for(;isspace(*str);++str);
            DB::Field::copyField(_structure[i], entry[i]);
            size_t comma = 0;
            for(;str[comma] != ',' && str[comma] != 0; ++comma);
            std::string val(str, comma);
            entry[i]->setValue(val.c_str());
            for(;!isspace(*str) && *str!=0;++str);  
        }
        return entry;
    }

    std::string DataBase::getReadableEntry(const Entry &entry){
        std::string res;
        for(size_t i = 0; i < entry.size(); ++i){
            res += _idx2name[i] + ": " + entry[i]->getValue() + ", ";
        }
        return res.substr(0, res.size()-2);
    }

    std::string DataBase::EntryToString(const Entry& entry){
        std::string res = std::to_string(entry.size()) + "$";
        res.back() = 0;
        for(size_t idx = 0; idx < entry.size(); ++idx){
            res += _idx2name[idx] + "$";
            res.back() = 0;
            res += entry[idx]->getValue() + "$";
            res.back() = 0;
        }
        return res;
    }

    Entry DataBase::StringToEntry(const char* str, bool use_default){
        char entry[1024];
        strncpy(entry, str, 1024);
        bool is_named = _filterEntry(entry);
        try{
            if(is_named){
                return _createNamed(entry, use_default);
            }
            else{
                return _createUnnamed(entry);
            }
        }
        catch(...){
            throw;
        }
    }

    bool DataBase::is_valid_entry(const std::string &entry){
        if(entry.find(':') != std::string::npos){
            return _checkNamed(entry);
        }
        else{
            return _checkUnnamed(entry);
        }
    }

    bool DataBase::is_valid_entry(const char *entry){
        std::string str;
        return is_valid_entry(std::string(entry));
    }
}

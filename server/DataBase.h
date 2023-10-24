#ifndef PEPENGU_DATABASE_H
#define PEPENGU_DATABASE_H

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include "Fields.h"


namespace DB{
    class structure{
    private:
        std::vector<std::unique_ptr<DB::Field>> _entry;
        std::map<std::string, size_t> _name2idx;
        size_t backup_frequency;

        void swap(structure &other){
            std::swap(_entry, other._entry);
            std::swap(_name2idx, other._name2idx);
        }

        static void copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to);

    public:
        structure(){}
        structure(const std::vector<std::unique_ptr<DB::Field>> &v){
            size_t idx = 0;
            for(const auto &el : _entry){
                _name2idx[el->getName()] = idx++;
            }

        }

        structure &operator=(const structure &other){
            _entry.resize(other._entry.size());
            for(size_t i = 0; i < other._entry.size(); ++i){
                copyField(other._entry[i], _entry[i]);
            }

            size_t idx = 0;
            for(const auto &el : _entry){
                _name2idx[el->getName()] = idx++;
            }

            return *this;
        }

        structure &operator=(structure &&other){
            swap(other);
            return *this;
        }

        size_t nameToIdx(const std::string &str);
        size_t nameToIdx(const char* str);

        friend DataBase;
    };


    class DataBase{
    private:
        structure _struct;
        std::map<std::string, std::string> _accounts;
        size_t _backup_frequency;
        size_t _backup_count;
        
        void parseConfig(std::ifstream &cfg);

        size_t processEntry(std::ifstream &cfg);
        size_t processUsers(std::ifstream &cfg);
        size_t processAddress(std::ifstream &cfg);
        size_t processFile(std::ifstream &cfg);
        size_t processBackupFrequency(std::ifstream &cfg);

    public:
        DataBase(const char* configFile): _backup_count(100){
            std::ifstream cfg(configFile);
            if(!cfg.is_open()){
                throw std::invalid_argument("Config file can not be oppened");
            }

            parseConfig(cfg);
        }
    };
}


#endif
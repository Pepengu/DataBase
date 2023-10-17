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
        std::vector<DB::Field*> _entry;
        std::map<std::string, size_t> _name2idx;

        void swap(structure &other){
            std::swap(_entry, other._entry);
            std::swap(_name2idx, other._name2idx);
        }

    public:
        structure(){}
        structure(const std::vector<DB::Field*> &v): _entry(v){
            size_t idx = 0;
            for(auto el : _entry){
                _name2idx[el->getName()] = idx++;
            }

        }

        structure &operator=(const structure &other){
            _entry = other._entry;
            size_t idx = 0;
            for(auto el : _entry){
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
    };


    class DataBase{
        structure _struct;
        
        void parseConfig(std::ifstream &cfg);

        void processEntry(std::ifstream &cfg);
        void processUsers(std::ifstream &cfg);
        void processAdress(std::ifstream &cfg);
        void processFile(std::ifstream &cfg);

    public:
        DataBase(const char* configFile){
            std::ifstream cfg(configFile);
            if(!cfg.is_open()){
                throw std::invalid_argument("Config file can not be oppened");
            }

            parseConfig(cfg);
        }
    };
}


#endif
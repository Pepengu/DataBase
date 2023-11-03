#ifndef PEPENGU_DATABASE_H
#define PEPENGU_DATABASE_H

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include "Fields.h"


namespace DB{
    class DataBase;
    class Entry{
    private:
        std::vector<std::unique_ptr<DB::Field>> _entry;
        std::map<std::string, size_t> _name2idx;

        void swap(Entry &other){
            std::swap(_entry, other._entry);
            std::swap(_name2idx, other._name2idx);
        }

        static void copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to);

    public:
        Entry(){}
        Entry(const std::vector<std::unique_ptr<DB::Field>> &v): _entry(v){}

        Entry &operator=(const Entry &other){
            _entry.resize(other._entry.size());
            for(size_t i = 0; i < other._entry.size(); ++i){
                copyField(other._entry[i], _entry[i]);
            }

            for(auto &[key, value] : other._name2idx){
                _name2idx[key] = value;
            }

            return *this;
        }

        Entry &operator=(Entry &&other){
            swap(other);
            return *this;
        }

        size_t nameToIdx(const std::string &str);
        size_t nameToIdx(const char* str);

        bool isSame();

        bool operator==(const Entry &other);
        bool operator<(const Entry &other);
        bool operator>(const Entry &other);

        friend DataBase;
    };


    class DataBase{
    private:
        Entry _structure;
        std::map<std::string, std::string> _accounts;
        size_t _backup_frequency;
        std::string _file;
        size_t _backup_count;
        std::vector<Entry> _entries;
        
        void _parseConfig(std::ifstream &cfg);

        size_t _processEntry(std::ifstream &cfg);
        size_t _processUsers(std::ifstream &cfg);
        size_t _processAddress(std::ifstream &cfg);
        size_t _processFile(std::ifstream &cfg);
        size_t _processBackupFrequency(std::ifstream &cfg);

    public:
        DataBase(const char* configFile): _backup_count(100){
            std::ifstream cfg(configFile);
            if(!cfg.is_open()){
                throw std::invalid_argument("Config file can not be oppened");
            }

            _parseConfig(cfg);
        }

        void addRecord();
        void save();
    };
}


#endif
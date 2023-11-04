#ifndef PEPENGU_DATABASE_H
#define PEPENGU_DATABASE_H

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include "Fields.h"


namespace DB{
    const std::string signature("PepenguDB");

    class DataBase;
    class Entry{
    public:
        std::vector<std::unique_ptr<DB::Field>> _entry;

        void swap(Entry &&other){
            std::swap(_entry, other._entry);
        }

        static void copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to);

    public:
        Entry(){}
        Entry(const std::vector<std::unique_ptr<DB::Field>> &v): _entry(v.size()){
            for(size_t i = 0; i < _entry.size(); ++i){
                copyField(v[i], _entry[i]);
            }
        }

        Entry(std::vector<std::unique_ptr<DB::Field>> &&v): _entry(v.size()){
            for(size_t i = 0; i < _entry.size(); ++i){
                _entry[i] = std::move(v[i]);
            }
        }

        Entry(const Entry &other): _entry(other._entry.size()){
            for(size_t field = 0; field < _entry.size(); ++field){
                copyField(other._entry[field], _entry[field]);
            }
        }
        
        Entry(Entry &&other){
            swap(std::move(other));
        }

        Entry &operator=(const Entry &other){
            _entry.resize(other._entry.size());
            for(size_t i = 0; i < other._entry.size(); ++i){
                copyField(other._entry[i], _entry[i]);
            }

            return *this;
        }

        Entry &operator=(Entry &&other){
            swap(std::move(other));
            return *this;
        }

        bool operator==(const Entry &other);
        bool operator<(const Entry &other);
        bool operator>(const Entry &other);

        friend DataBase;
    };


    class DataBase{
    public:
        Entry _structure;
        std::map<std::string, std::string> _accounts;
        std::map<std::string, size_t> _name2idx;
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
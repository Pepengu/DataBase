#ifndef PEPENGU_DATABASE_H
#define PEPENGU_DATABASE_H

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <iostream>
#include "Fields.h"


namespace DB{
    const std::string signature("PepenguDB");

    class Entry{
    private:
        using fields_vector = std::vector<std::unique_ptr<DB::Field>>;
        fields_vector _entry;

        void swap(Entry &&other){
            std::swap(_entry, other._entry);
        }

    public:
        using iterator = fields_vector::iterator;
        using const_iterator = fields_vector::const_iterator;
        
        iterator begin() { return _entry.begin(); }
        iterator end() { return _entry.end(); }
        const_iterator begin() const { return _entry.begin(); }
        const_iterator end() const { return _entry.end(); }
        const_iterator cbegin() const { return _entry.cbegin(); }
        const_iterator cend() const { return _entry.cend(); }


        Entry(){}
        Entry(const std::vector<std::unique_ptr<DB::Field>> &v): _entry(v.size()){
            for(size_t i = 0; i < _entry.size(); ++i){
                Field::copyField(v[i], _entry[i]);
            }
        }

        Entry(std::vector<std::unique_ptr<DB::Field>> &&v): _entry(v.size()){
            for(size_t i = 0; i < _entry.size(); ++i){
                _entry[i] = std::move(v[i]);
            }
        }

        Entry(const Entry &other): _entry(other._entry.size()){
            for(size_t field = 0; field < _entry.size(); ++field){
                Field::copyField(other._entry[field], _entry[field]);
            }
        }
        
        Entry(Entry &&other){
            swap(std::move(other));
        }

        Entry &operator=(const Entry &other){
            _entry.resize(other._entry.size());
            for(size_t i = 0; i < other._entry.size(); ++i){
                Field::copyField(other._entry[i], _entry[i]);
            }

            return *this;
        }

        Entry &operator=(Entry &&other){
            swap(std::move(other));
            return *this;
        }

        std::unique_ptr<DB::Field> &operator [](const size_t idx){
            return _entry[idx];
        }

        const std::unique_ptr<DB::Field> &operator [](const size_t idx) const{
            return _entry[idx];
        }

        inline size_t size() const{
            return _entry.size();
        }

        inline void push_back(const std::unique_ptr<DB::Field> &field){
            _entry.resize(_entry.size()+1);
            Field::copyField(field, _entry.back());
        }

        inline void resize(size_t size){
            _entry.resize(size);
        }

        bool operator==(const Entry &other);
        bool operator<(const Entry &other);
        bool operator>(const Entry &other);
    };


    class DataBase{
    private:
        using entries_vector = std::vector<Entry>;

        Entry _structure;
        std::map<std::string, std::string> _accounts;
        std::map<std::string, size_t> _name2idx;
        size_t _backup_frequency;
        std::string _file;
        size_t _backup_count;
        entries_vector _entries;
        
        void _parseConfig(std::ifstream &cfg);

        size_t _processEntry(std::ifstream &cfg);
        size_t _processUsers(std::ifstream &cfg);
        size_t _processAddress(std::ifstream &cfg);
        size_t _processFile(std::ifstream &cfg);
        size_t _processBackupFrequency(std::ifstream &cfg);

    public:

        using iterator = entries_vector::iterator;
        using const_iterator = entries_vector::const_iterator;

        iterator begin() { return _entries.begin(); }
        iterator end() { return _entries.end(); }
        const_iterator begin() const { return _entries.begin(); }
        const_iterator end() const { return _entries.end(); }
        const_iterator cbegin() const { return _entries.cbegin(); }
        const_iterator cend() const { return _entries.cend(); }

        DataBase(const char* configFile): _backup_count(100){
            std::ifstream cfg(configFile);
            _entries.reserve(2);
            if(!cfg.is_open()){
                throw std::invalid_argument("Config file can not be oppened");
            }

            _parseConfig(cfg);
        }

        void addRecord(const Entry &entry);
        Entry &operator[](size_t idx){
            if(idx >= _entries.size()) {
                throw std::invalid_argument("Entry with id " + std::to_string(idx) + " does not exist");
            }
            return _entries[idx]; 
        }
        void remove(size_t idx);

        void save();
    };
}


#endif
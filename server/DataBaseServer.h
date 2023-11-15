#ifndef PEPENGU_DATABASE_H
#define PEPENGU_DATABASE_H

#include "DataBase.h"
#include <fstream>
#include <map>
#include <stdexcept>

namespace DB{
    class DataBaseServer : DataBase{
    private:
        using entries_vector = std::vector<Entry>;

        std::map<std::string, std::string> _accounts;
        size_t _backup_frequency;
        std::string _file;
        size_t _backup_count;
        entries_vector _entries;


        size_t _processUsers(std::ifstream &cfg);
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

        DataBaseServer(const char* configFile);

        void addRecord(const Entry &entry);
        Entry &operator[](size_t idx);
        void remove(size_t idx);

        void save();
    };
}


#endif
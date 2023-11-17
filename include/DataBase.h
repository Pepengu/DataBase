#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <functional>

#include "Fields.h"

namespace DB{
    /**Entry is a vector of fields
     * Does not store names of the fields
    */
    class Entry{
    private:
        using fields_vector = std::vector<std::unique_ptr<DB::Field>>;
        fields_vector _entry;

        void swap(Entry &&other);

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
        Entry(const std::vector<std::unique_ptr<DB::Field>> &v);
        Entry(std::vector<std::unique_ptr<DB::Field>> &&v);
        Entry(const Entry &other);
        Entry(Entry &&other);

        Entry &operator=(const Entry &other);
        Entry &operator=(Entry &&other);

        std::unique_ptr<DB::Field> &operator [](const size_t idx);
        const std::unique_ptr<DB::Field> &operator [](const size_t idx) const;

        size_t size() const;
        void push_back(const std::unique_ptr<DB::Field> &field);
        void resize(size_t size);
    };

    /**Base class of a Data Base
    */
    class DataBase{
    protected:
        std::map<std::string, std::function<size_t(std::ifstream &cfg)>> _configMap{
            {"entry", std::bind(&DB::DataBase::_processEntry, this, std::placeholders::_1)}
        };

        Entry _structure;
        std::map<std::string, size_t> _name2idx;

        std::string structureToString();

        void _parseConfig(std::ifstream &cfg);

        size_t _processEntry(std::ifstream &cfg);
    };
}
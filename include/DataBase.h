#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <functional>

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
    };

    class DataBase{
    protected:
        std::map<std::string, std::function<size_t(std::ifstream &cfg)>> _configMap{
            {"entry", std::bind(&DB::DataBase::_processEntry, this, std::placeholders::_1)},
            {"address", std::bind(&DB::DataBase::_processAddress, this, std::placeholders::_1)}
        };

        Entry _structure;
        std::map<std::string, size_t> _name2idx;

        void _parseConfig(std::ifstream &cfg);

        size_t _processEntry(std::ifstream &cfg);
        size_t _processAddress(std::ifstream &cfg);
    };
}
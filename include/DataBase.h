#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <functional>
#include "Logger.h"

#include "Fields.h"

namespace DB{
    const int PORT = 8080;
    /**Entry is a vector of fields
    */
    using Entry = std::vector<std::unique_ptr<DB::Field>>;
    std::vector<std::string> getValue(const Entry &entry);
    std::vector<FIELDS> getTypes(const Entry &entry);

    enum REQUEST_TYPE{
        connection,
        add,
        remove,
        edit,
        dump,
        get,
        filter,
        stop,
        save
    };

    /**Base class of a Data Base
    */
    class DataBase{
    protected:
        Logger log;

        std::map<std::string, std::function<size_t(std::ifstream &cfg)>> _configMap{
            {"entry", std::bind(&DB::DataBase::_processEntry, this, std::placeholders::_1)}
        };

        Entry _structure;
        std::map<std::string, size_t> _name2idx;
        std::vector<std::string> _idx2name;

        std::string _structureToString();

        void _parseConfig(const char* configFile);

        size_t _processEntry(std::ifstream &cfg);

        bool _checkNamed(const std::string &entry);
        bool _checkUnnamed(const std::string &entry);
    
    public:
        bool is_valid_entry(const std::string &entry);
        bool is_valid_entry(const char *entry);
    };

    enum STATUS : unsigned char{
        success = 0x00,
        connection_success,
        save_success,
        close_success,
        addition_success,
        removal_success,
        edition_success,

        connention_error = 0x10,
        username_invalid,
        password_invalid,
        structure_differ,

        addition_error = 0x20,
        not_enough_fields,

        edit_error = 0x30,
        field_does_not_exist,

        filter_error = 0x40,

        save_error = 0x50,
        save_file_inaccessible,

        causeMask = 0xF,
        typeMask  = 0xF0
    };
}
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

    enum requestType{
        connection,
        add,
        remove,
        edit,
        dump,
        filter
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
    };

    enum status : unsigned char{
        success = 0x00,
        connection_success,

        connention_error = 0x10,
        username_invalid,
        password_invalid,
        structure_differ,

        addition_error = 0x20,

        edit_error = 0x30,

        filter_error = 0x40,

        causeMask = 0xF,
        typeMask  = 0xF0
    };
}
#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <functional>
#include "Logger.h"

#include "Fields.h"

namespace DB{
    /**Entry is a vector of fields
    */
    using Entry = std::vector<std::unique_ptr<DB::Field>>;
    std::vector<std::string> getValue(const Entry &entry);
    std::vector<FIELDS> getTypes(const Entry &entry);

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
}
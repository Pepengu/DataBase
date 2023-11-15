#ifndef PEPENGU_FIELDS_H
#define PEPENGU_FIELDS_H

#include <string>
#include <cstring>
#include <memory>
#include <map>
#include <type_traits>

namespace DB
{

    constexpr int DEFAULT_INT = 0;
    constexpr double DEFAULT_DOUBLE = 0;
    constexpr bool DEFAULT_BOOl = 0;
    constexpr char DEFAULT_STRING[1]{0};

    class Field
    {
    public:
        Field(){}
        ~Field(){}

        Field(const Field &other){}
        Field(Field &&other){}

        static void copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to);

        virtual inline std::string getValue() const = 0;
    };

    template <typename Integer,
              typename = std::enable_if<std::is_integral<Integer>::value>>
    class NumberField : public Field
    {
        Integer value;

        void swap(NumberField<Integer> &other)
        {
            std::swap(value, other.value);
        }

    public:
        NumberField() : value(static_cast<Integer>(DEFAULT_INT)) {}

        NumberField(Integer val) : value(val) {}

        NumberField(const NumberField &other) : value(other.value) {}
        NumberField(NumberField &&other) noexcept { swap(other); }

        inline std::string getValue() const
        {
            return std::to_string(value);
        }

        inline void setValue(Integer val)
        {
            value = val;
        }
    };

    class StringField : public Field
    {
        char *value;

        void swap(StringField &other)
        {
            std::swap(value, other.value);
        }

    public:
        StringField() {memcpy(value, DEFAULT_STRING, sizeof(DEFAULT_STRING));}

        StringField(const char *valstr) : value(strcpy(new char[strlen(valstr) + 1], valstr)) {}

        StringField(const std::string &valstr): value(strcpy(new char[valstr.size() + 1], valstr.c_str())) {}

        StringField(const StringField &other): StringField(other.getValue()){}

        StringField(StringField &&other) : value(nullptr) { swap(other); }

        ~StringField()
        {
            delete value;
        }

        inline std::string getValue() const
        {
            return value == nullptr ? std::string("") : std::string(value);
        }

        inline void setValue(const std::string &val)
        {
            strncpy(value = new char[val.size() + 1]{0}, val.c_str(), val.size());
        }

        inline void setValue(const char *val)
        {
            strncpy(value = new char[strlen(val)], val, strlen(val));
        }
    };

    class BoolField : public Field
    {
        bool value;

        void swap(BoolField &other)
        {
            std::swap(value, other.value);
        }

    public:
        BoolField() : value(DEFAULT_BOOl) {}

        BoolField(bool val) : value(val) {}

        BoolField(const BoolField &other) : value(other.value) {}
        BoolField(BoolField &&other) noexcept { swap(other); }

        inline std::string getValue() const
        {
            return value ? "TRUE" : "FALSE";
        }

        inline void setValue(bool val)
        {
            value = val;
        }
    };

    class DoubleField : public Field
    {
        double value;

        void swap(DoubleField &other)
        {
            std::swap(value, other.value);
        }

    public:
        DoubleField() : value(DEFAULT_DOUBLE) {}

        DoubleField(double val) : value(val) {}

        DoubleField(const DoubleField &other) : value(other.value) {}
        DoubleField(DoubleField &&other) noexcept { swap(other); }

        inline std::string getValue() const
        {
            return std::to_string(value);
        }

        inline void setValue(double val)
        {
            value = val;
        }
    };

    static std::map<std::string, uint8_t> _str2Idx{
        {"byte", 0},
        {"short int", 1},
        {"int", 2},
        {"long int", 3},
        {"unsigned byte", 4},
        {"unsigned short int", 5},
        {"unsigned int", 6},
        {"unsigned long int", 7},
        {"bool", 8},
        {"double", 9},
        {"string", 10}};

    static std::map<size_t, uint8_t> _hash2Idx{
        {typeid(NumberField<int8_t>).hash_code(), 0},
        {typeid(NumberField<int16_t>).hash_code(), 1},
        {typeid(NumberField<int32_t>).hash_code(), 2},
        {typeid(NumberField<int64_t>).hash_code(), 3},
        {typeid(NumberField<uint8_t>).hash_code(), 4},
        {typeid(NumberField<uint16_t>).hash_code(), 5},
        {typeid(NumberField<uint32_t>).hash_code(), 6},
        {typeid(NumberField<uint64_t>).hash_code(), 7},
        {typeid(BoolField).hash_code(), 8},
        {typeid(DoubleField).hash_code(), 9},
        {typeid(StringField).hash_code(), 10}};

    
    inline std::unique_ptr<DB::Field> createField(const double value){
        return std::make_unique<DB::DoubleField>(value);
    }

    inline std::unique_ptr<DB::Field> createField(const bool value){
        return std::make_unique<DB::BoolField>(value);
    }

    inline std::unique_ptr<DB::Field> createField(const std::string &value){
        return std::make_unique<DB::StringField>(value);
    }

    inline std::unique_ptr<DB::Field> createField(const char* value){
        return std::make_unique<DB::StringField>(value);
    }

    inline std::unique_ptr<DB::Field> createField(char* value){
        return std::make_unique<DB::StringField>(value);
    }

    template <
            typename T,
            typename = std::enable_if<std::is_integral<T>::value>>
    inline std::unique_ptr<DB::Field> createField(const T value){
        return std::make_unique<DB::NumberField<T>>(value);
    }
};

#endif
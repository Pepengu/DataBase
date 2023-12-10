#ifndef PEPENGU_FIELDS_H
#define PEPENGU_FIELDS_H

#include <string>
#include <cstring>
#include <memory>
#include <map>
#include <type_traits>

namespace DB
{
    //Enum for more convinients fields logic
    enum FIELDS : char{
        BYTE,
        SHORT,
        INT,
        LONG,
        UBYTE,
        USHORT,
        UINT,
        ULONG,
        BOOL,
        DOUBLE,
        STRING
    };

    //Values that are used if field is created without specifying it's value
    constexpr int8_t DEFAULT_BYTE = 0;
    constexpr int16_t DEFAULT_SHORT = 0;
    constexpr int32_t DEFAULT_INT = 0;
    constexpr int64_t DEFAULT_LONG = 0;
    constexpr uint8_t DEFAULT_UBYTE = 0;
    constexpr uint16_t DEFAULT_USHORT = 0;
    constexpr uint32_t DEFAULT_UINT = 0;
    constexpr uint64_t DEFAULT_ULONG = 0;
    constexpr double DEFAULT_DOUBLE = 0;
    constexpr bool DEFAULT_BOOl = 0;
    constexpr char DEFAULT_STRING[1]{0};

    class Field{
    public:
        Field(){}
        ~Field(){}

        Field(const Field &other){}
        Field(Field &&other){}

        static void copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to);

        virtual inline std::string getValue() const = 0;
        virtual inline void setValue(const char* str) = 0;
    };

    /**Number field is used to store integer values
     * 
     * Field is created using template that allows only integral types to prevent confusing it with Double field
     * */
    template <typename Integer, typename = std::enable_if<std::is_integral<Integer>::value>>
    class NumberField : public Field
    {
        Integer value;

        void swap(NumberField<Integer> &other){std::swap(value, other.value);}

    public:
        NumberField() : value(static_cast<Integer>(DEFAULT_INT)) {}
        NumberField(Integer val) : value(val) {}
        NumberField(const NumberField &other) : value(other.value) {}
        NumberField(NumberField &&other) noexcept { swap(other); }

        inline std::string getValue() const{return std::to_string(value);}
        inline void setValue(const char* str){
            if(std::is_same<Integer, uint64_t>::value){
                value = std::stoull(std::string(str));
            }
            else{
                value = static_cast<Integer>(std::stoll(std::string(str)));
            }
        }
        inline void setValue(Integer val){value = val;}
    };

    /**String field is used to store text values (both string and char)
     * 
     * Field is storing vales as dynamic array of char type
    */
    class StringField : public Field
    {
        char *value;

        void swap(StringField &other){std::swap(value, other.value);}

    public:
        StringField(): value(new char[sizeof(DEFAULT_STRING)]) {memcpy(value, DEFAULT_STRING, sizeof(DEFAULT_STRING));}
        StringField(const char *valstr) : value(strcpy(new char[strlen(valstr) + 1], valstr)) {}
        StringField(const std::string &valstr): value(strcpy(new char[valstr.size() + 1], valstr.c_str())) {}
        StringField(const StringField &other): StringField(other.getValue()){}
        StringField(StringField &&other) : value(nullptr) { swap(other); }

        ~StringField(){delete[] value;}

        inline std::string getValue() const{return value == nullptr ? std::string("") : std::string(value);}

        inline void setValue(const std::string &val){
            value = new char[val.size() + 1];
            strncpy(value, val.c_str(), val.size());
            value[val.size()] = 0;
        }
        inline void setValue(const char *val){
            size_t length = strlen(val);
            value = new char[length + 1];
            strncpy(value, val, length+1);
            value[length] = 0;
        }
    };

    /**Bool field is used to store logical values (true or false) 
    */
    class BoolField : public Field
    {
        bool value;

        void swap(BoolField &other){std::swap(value, other.value);}

    public:
        BoolField() : value(DEFAULT_BOOl) {}
        BoolField(bool val) : value(val) {}
        BoolField(const BoolField &other) : value(other.value) {}
        BoolField(BoolField &&other) noexcept { swap(other); }

        inline std::string getValue() const{return value ? "TRUE" : "FALSE";}
        inline void setValue(const char* str){value = str[0];}
        inline void setValue(bool val){value = val;}
    };

    /**Double field is used to store real numbers
     * 
     * Do not confuse it with Number field
     * If floating point values are not needed use Number field to increase precision of larger values
    */
    class DoubleField : public Field
    {
        double value;

        void swap(DoubleField &other){std::swap(value, other.value);}

    public:
        DoubleField() : value(DEFAULT_DOUBLE) {}
        DoubleField(double val) : value(val) {}
        DoubleField(const DoubleField &other) : value(other.value) {}
        DoubleField(DoubleField &&other) noexcept { swap(other); }

        inline std::string getValue() const{return std::to_string(value);}
        inline void setValue(const char* str){value = atof(str);}

        inline void setValue(double val){value = val;}
    };

    /**USE ONLY IF YOU ARE SURE ABOUT WHAT YOU ARE DOING
     * NOT NEEDED IF YOU ARE NOT IMPLEMENTING NEW FIELD TYPES
    */
    static std::map<std::string, FIELDS> _str2Type{
        {"byte", BYTE},
        {"short int", SHORT},
        {"int", INT},
        {"long int", LONG},
        {"unsigned byte", UBYTE},
        {"unsigned short int", USHORT},
        {"unsigned int", UINT},
        {"unsigned long int", ULONG},
        {"bool", BOOL},
        {"double", DOUBLE},
        {"string", STRING}
    };

    /**USE ONLY IF YOU ARE SURE ABOUT WHAT YOU ARE DOING
     * NOT NEEDED IF YOU ARE NOT IMPLEMENTING NEW FIELD TYPES
    */
    static std::map<size_t, FIELDS> _hash2Type{
        {typeid(NumberField<int8_t>).hash_code(), BYTE},
        {typeid(NumberField<int16_t>).hash_code(), SHORT},
        {typeid(NumberField<int32_t>).hash_code(), INT},
        {typeid(NumberField<int64_t>).hash_code(), LONG},
        {typeid(NumberField<uint8_t>).hash_code(), UBYTE},
        {typeid(NumberField<uint16_t>).hash_code(), USHORT},
        {typeid(NumberField<uint32_t>).hash_code(), UINT},
        {typeid(NumberField<uint64_t>).hash_code(), ULONG},
        {typeid(BoolField).hash_code(), BOOL},
        {typeid(DoubleField).hash_code(), DOUBLE},
        {typeid(StringField).hash_code(), STRING}
    };

    /**Function used to create field whithout without worying about it's stored type 
    */
    template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
    inline std::unique_ptr<DB::Field> createField(const T value){return std::make_unique<DB::NumberField<T>>(value);}
    inline std::unique_ptr<DB::Field> createField(const double value){return std::make_unique<DB::DoubleField>(value);}
    inline std::unique_ptr<DB::Field> createField(const bool value){return std::make_unique<DB::BoolField>(value);}
    inline std::unique_ptr<DB::Field> createField(const std::string &value){return std::make_unique<DB::StringField>(value);}
    inline std::unique_ptr<DB::Field> createField(const char* value){return std::make_unique<DB::StringField>(value);}
    inline std::unique_ptr<DB::Field> createField(char* value){return std::make_unique<DB::StringField>(value);}
}

#endif
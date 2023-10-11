#ifndef PEPENGU_FIELDS_H
#define PEPENGU_FIELDS_H
#include <string>
#include <cstring>

namespace DB{
    class Field{
    protected:
        char* name;
    
    public:
        Field(): name(nullptr){}
        Field(const char* str): name(strcpy(new char[strlen(str)+1], str)){}
        Field(const std::string &str): name(strcpy(new char[str.size()+1], str.c_str())){}

        ~Field(){
            delete name;
        }

        virtual inline std::string getValue() const = 0;
        inline std::string getName(){return std::string(name);}
    };


    template<typename T>
    class NumberField : public Field{
        T value;

        void swap(NumberField<T> &other){
            std::swap(name, other.name);
            std::swap(value, other.value);
        }

    public:
        NumberField(const char* str): Field(str), value(0){}
        NumberField(const std::string &str): Field(str), value(0){}
        
        NumberField(const char* str, T val): Field(str), value(val){}
        NumberField(const std::string &str, T val): Field(str), value(val){}

        NumberField(const NumberField &other): Field(other.name), value(other.value){}
        NumberField(NumberField &&other) noexcept{swap(other);}

        inline std::string getValue() const{
            return std::to_string(value);
        }
    };

    class StringField : public Field{
        char* value;

        void swap(StringField &other){
            std::swap(name, other.name);
            std::swap(value, other.value);
        }

        StringField(const char* str): Field(str), value(nullptr){}
        StringField(const std::string &str): Field(str), value(nullptr){}
        
        StringField(const char* namestr, const char* valstr): Field(namestr), value(strcpy(new char[strlen(valstr)+1], valstr)){}
        StringField(const std::string &namestr, const char* valstr): Field(namestr), value(strcpy(new char[strlen(valstr)+1], valstr)){}

        StringField(const char* namestr, const std::string &valstr): Field(namestr), value(strcpy(new char[valstr.size()+1], valstr.c_str())){}
        StringField(const std::string &namestr, const std::string &valstr): Field(namestr), value(strcpy(new char[valstr.size()+1], valstr.c_str())){}

        StringField(const StringField &other): Field(other.name), value(strcpy(new char[strlen(other.value)+1], other.value)){}

        StringField(StringField &&other): value(nullptr) {swap(other);}

        ~StringField(){
            delete value;
        }

    public:
        inline std::string getValue() const{
            return value == nullptr ? std::string("") : std::string(value);
        }
    };

    class BoolField : public Field{
        bool value;

        void swap(BoolField &other){
            std::swap(name, other.name);
            std::swap(value, other.value);
        }

    public:
        BoolField(const char* str): Field(str), value(false){}
        BoolField(const std::string &str): Field(str), value(false){}
        
        BoolField(const char* str, bool val): Field(str), value(val){}
        BoolField(const std::string &str, bool val): Field(str), value(val){}

        BoolField(const BoolField &other): Field(other.name), value(other.value){}
        BoolField(BoolField &&other) noexcept{swap(other);}

        inline std::string getValue() const{
            return value ? "TRUE" : "FALSE";
        }
    };
};

#endif
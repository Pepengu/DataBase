#include <vector>
#include <string>
#include <cstring>

namespace DB{
    class Field{
    
    public:
        virtual std::string getValue() const = 0;
    };


    //Integer type fields
    class ByteIntField : Field{
        int8_t value;

        ByteIntField(): value(0){}
        ByteIntField(int8_t val): value(val){}

        ByteIntField(const ByteIntField &other): value(other.value){}
        ByteIntField(ByteIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class ShortIntField : Field{
        int16_t value;

        ShortIntField(): value(0){}
        ShortIntField(int16_t val): value(val){}

        ShortIntField(const ShortIntField &other): value(other.value){}
        ShortIntField(ShortIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class IntField : Field{
        int32_t value;

        IntField(): value(0){}
        IntField(int32_t val): value(val){}

        IntField(const IntField &other): value(other.value){}
        IntField(IntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class LongIntField : Field{
        int64_t value;

        LongIntField(): value(0){}
        LongIntField(int64_t val): value(val){}

        LongIntField(const LongIntField &other): value(other.value){}
        LongIntField(LongIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class UByteIntField : Field{
        uint8_t value;

        UByteIntField(): value(0){}
        UByteIntField(uint8_t val): value(val){}

        UByteIntField(const UByteIntField &other): value(other.value){}
        UByteIntField(UByteIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class UShortIntField : Field{
        uint16_t value;

        UShortIntField(): value(0){}
        UShortIntField(uint16_t val): value(val){}

        UShortIntField(const UShortIntField &other): value(other.value){}
        UShortIntField(UShortIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class UIntField : Field{
        uint32_t value;

        UIntField(): value(0){}
        UIntField(uint32_t val): value(val){}

        UIntField(const UIntField &other): value(other.value){}
        UIntField(UIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };

    class ULongIntField : Field{
        uint64_t value;

        ULongIntField(): value(0){}
        ULongIntField(uint64_t val): value(val){}

        ULongIntField(const ULongIntField &other): value(other.value){}
        ULongIntField(ULongIntField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };


    class StringField : Field{
        char* value;

        StringField(): value(nullptr){}

        StringField(const char* val){
            size_t n = strlen(val);
            value = new char[n];
            memcpy(value, val, n);
        }

        StringField(const std::string val){
            size_t n = val.size();
            value = new char[n+1];
            for(int i = 0; i < n; ++i){
                value[i] = val[i];
            }
            value[n] = '\0';
        }

        StringField(const StringField &other){
            size_t n = strlen(other.value);
            value = new char[n];
            memcpy(value, other.value, n);
        }

        StringField(StringField &&other): value(nullptr){
            std::swap(value, other.value);
        }

        ~StringField(){
            delete value;
        }

    public:
        std::string getValue() const{
            return value == nullptr ? std::string("") : std::string(value);
        }
    };

    class BoolField : Field{
        bool value;

        BoolField(): value(false){}
        BoolField(bool val): value(val){}

        BoolField(const BoolField &other): value(other.value){}
        BoolField(BoolField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return value ? "TRUE" : "FALSE";
        }
    };

    class DoubleField : Field{
        double value;

        DoubleField(): value(0){}
        DoubleField(double val): value(val){}

        DoubleField(const DoubleField &other): value(other.value){}
        DoubleField(DoubleField &&other){
            std::swap(value, other.value);
        }

    public:
        std::string getValue() const{
            return std::to_string(value);
        }
    };
};
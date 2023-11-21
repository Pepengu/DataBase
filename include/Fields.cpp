#include "Fields.h"
#include <stdexcept>

void DB::Field::copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to){
    if(from.get() != nullptr){
        auto &ptr = *from.get();
        size_t hash = typeid(ptr).hash_code();
        
        if(DB::_hash2Idx.find(hash) == DB::_hash2Idx.end()){
            throw std::invalid_argument("Unknown field type occured");
        }

        switch (DB::_hash2Idx[hash]){
            case BYTE:
                to = std::make_unique<DB::NumberField<int8_t>>(*dynamic_cast<const DB::NumberField<int8_t>*>(&ptr));
                break;
            case SHORT:
                to = std::make_unique<DB::NumberField<int16_t>>(*dynamic_cast<const DB::NumberField<int16_t>*>(&ptr));
                break;
            case INT:
                to = std::make_unique<DB::NumberField<int32_t>>(*dynamic_cast<const DB::NumberField<int32_t>*>(&ptr));
                break;
            case LONG:
                to = std::make_unique<DB::NumberField<int64_t>>(*dynamic_cast<const DB::NumberField<int64_t>*>(&ptr));
                break;
            case UBYTE:
                to = std::make_unique<DB::NumberField<uint8_t>>(*dynamic_cast<const DB::NumberField<uint8_t>*>(&ptr));
                break;
            case USHORT:
                to = std::make_unique<DB::NumberField<uint16_t>>(*dynamic_cast<const DB::NumberField<uint16_t>*>(&ptr));
                break;
            case UINT:
                to = std::make_unique<DB::NumberField<uint32_t>>(*dynamic_cast<const DB::NumberField<uint32_t>*>(&ptr));
                break;
            case ULONG:
                to = std::make_unique<DB::NumberField<uint64_t>>(*dynamic_cast<const DB::NumberField<uint64_t>*>(&ptr));
                break;
            case BOOL:
                to = std::make_unique<DB::BoolField>(*dynamic_cast<const DB::BoolField*>(&ptr));
                break;
            case DOUBLE:
                to = std::make_unique<DB::DoubleField>(*dynamic_cast<const DB::DoubleField*>(&ptr));
                break;
            case STRING:
                to = std::make_unique<DB::StringField>(*dynamic_cast<const DB::StringField*>(&ptr));
                break;
        }
    }
    else{
        throw std::invalid_argument("Empty field occured");
    }
}
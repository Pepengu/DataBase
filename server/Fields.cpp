#include "Fields.h"
#include <stdexcept>

void DB::Field::copyField(const std::unique_ptr<DB::Field> &from, std::unique_ptr<DB::Field> &to){
    if(from.get() != nullptr){
        auto &ptr = *from.get();
        switch (DB::_hash2Idx[typeid(ptr).hash_code()]){
            case 0:
                to = std::make_unique<DB::NumberField<int8_t>>(*dynamic_cast<const DB::NumberField<int8_t>*>(&ptr));
                break;
            case 1:
                to = std::make_unique<DB::NumberField<int16_t>>(*dynamic_cast<const DB::NumberField<int16_t>*>(&ptr));
                break;
            case 2:
                to = std::make_unique<DB::NumberField<int32_t>>(*dynamic_cast<const DB::NumberField<int32_t>*>(&ptr));
                break;
            case 3:
                to = std::make_unique<DB::NumberField<int64_t>>(*dynamic_cast<const DB::NumberField<int64_t>*>(&ptr));
                break;
            case 4:
                to = std::make_unique<DB::NumberField<uint8_t>>(*dynamic_cast<const DB::NumberField<uint8_t>*>(&ptr));
                break;
            case 5:
                to = std::make_unique<DB::NumberField<uint16_t>>(*dynamic_cast<const DB::NumberField<uint16_t>*>(&ptr));
                break;
            case 6:
                to = std::make_unique<DB::NumberField<uint32_t>>(*dynamic_cast<const DB::NumberField<uint32_t>*>(&ptr));
                break;
            case 7:
                to = std::make_unique<DB::NumberField<uint64_t>>(*dynamic_cast<const DB::NumberField<uint64_t>*>(&ptr));
                break;
            case 8:
                to = std::make_unique<DB::BoolField>(*dynamic_cast<const DB::BoolField*>(&ptr));
                break;
            case 9:
                to = std::make_unique<DB::DoubleField>(*dynamic_cast<const DB::DoubleField*>(&ptr));
                break;
            case 10:
                to = std::make_unique<DB::StringField>(*dynamic_cast<const DB::StringField*>(&ptr));
                break;
            default:
                throw std::invalid_argument("Unknown field type occured");
                break;
        }
    }
    else{
        throw std::invalid_argument("Empty field occured");
    }
}
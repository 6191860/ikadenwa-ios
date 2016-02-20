//
//  any.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/10.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "any.h"
#include "env.h"
#include "string.h"
#include "array.h"
#include "map.h"
#include "json.h"

namespace nwr {
    Any::Any(): type_(Type::Null), value_(nullptr) {}
    Any::Any(const Any & copy): type_(Type::Null), value_(nullptr) {
        *this = copy;
    }
    Any::Any(Any && move): type_(Type::Null), value_(nullptr) {
        *this = move;
    }
    
    Any::Any(std::nullptr_t value): type_(Type::Null), value_(nullptr) {}
    
    Any::Any(bool value): type_(Type::Boolean), value_(std::make_shared<bool>(value)) {}
    
    Any::Any(int value): Any(static_cast<double>(value)){}
    Any::Any(double value): type_(Type::Number), value_(std::make_shared<double>(value)) {}
    
    Any::Any(const char * value): Any(std::string(value)) {};
    Any::Any(const std::string & value): type_(Type::String), value_(std::make_shared<std::string>(value)) {}
    
    Any::Any(const Data & value): Any(std::make_shared<Data>(value)){}
    Any::Any(const DataPtr & value): type_(Type::Data), value_(value) {}
    
    Any::Any(const ArrayType & value):
    type_(Type::Array), value_(std::make_shared<ArrayType>(value)) {}

    Any::Any(const ObjectType & value):
    type_(Type::Object), value_(std::make_shared<ObjectType>(value)) {}
    
    Any::Any(const AnyFuncPtr & value):
    type_(Type::Function), value_(value) {}
    
    Any::Any(const PointerType & value):
    type_(Type::Pointer), value_(value) {}
    
    Any::Type Any::type() const {
        return type_;
    }
    
    int Any::count() const {
        switch (type()) {
            case Type::Array:
                return static_cast<int>(AsArray()->size());
            case Type::Object:
                return static_cast<int>(AsObject()->size());
            default:
                return 0;
        }
    }
    
    std::vector<std::string> Any::keys() const {
        if (type() == Type::Object) {
            return Keys(*inner_object());
        } else {
            return std::vector<std::string>();
        }
    }
    
    Any::operator bool() const {
        return type() != Type::Null;
    }
    
    Optional<bool> Any::AsBoolean() const {
        if (type() == Type::Boolean) {
            return Some(*std::static_pointer_cast<bool>(value_));
        } else {
            return None();
        }
    }
    Optional<int> Any::AsInt() const {
        return AsDouble().Map<int>([](double x) { return static_cast<int>(x); });
    }
    Optional<double> Any::AsDouble() const {
        if (type() == Type::Number) {
            return Some(*std::static_pointer_cast<double>(value_));
        } else {
            return None();
        }
    }
    Optional<std::string> Any::AsString() const {
        if (type() == Type::String) {
            return Some(*std::static_pointer_cast<std::string>(value_));
        } else {
            return None();
        }
    }
    Optional<DataPtr> Any::AsData() const {
        if (type() == Type::Data) {
            return Some(std::static_pointer_cast<Data>(value_));
        } else {
            return None();
        }
    }
    Optional<Any::ArrayType> Any::AsArray() const {
        auto array = inner_array();
        return array ? Some(*array) : None();
    }
    Optional<Any::ObjectType> Any::AsObject() const {
        auto dict = inner_object();
        return dict ? Some(*dict) : None();
    }
    
    Optional<AnyFuncPtr> Any::AsFunction() const {
        if (type() == Type::Function) {
            return Some(std::static_pointer_cast<AnyFunc>(value_));
        } else {
            return None();
        }
    }
    
    Optional<Any::PointerType> Any::AsPointer() const {
        if (type() == Type::Pointer) {
            return Some(value_);
        } else {
            return None();
        }
    }
    
    Any & Any::operator= (const Any & copy) {
        type_ = copy.type_;
        
        switch (type_) {
            case Type::Null:
                value_ = nullptr;
                break;
            case Type::Boolean:
                value_ = std::make_shared<bool>(*copy.AsBoolean());
                break;
            case Type::Number:
                value_ = std::make_shared<double>(*copy.AsDouble());
                break;
            case Type::String:
                value_ = std::make_shared<std::string>(*copy.AsString());
                break;
            case Type::Data:
            case Type::Array:
            case Type::Object:
            case Type::Function:
            case Type::Pointer:
                value_ = copy.value_;
                break;
        }
        
        return *this;
    }
    Any & Any::operator= (Any && move) {
        type_ = move.type_;
        value_ = move.value_;
        move.type_ = Type::Null;
        move.value_ = nullptr;
        return *this;
    }
    
    bool Any::operator== (const Any & cmp) const {
        switch (type_) {
            case Type::Null:
                return cmp.type() == Type::Null;
            case Type::Boolean:
                return AsBoolean() == cmp.AsBoolean();
            case Type::Number:
                return AsDouble() == cmp.AsDouble();
            case Type::String:
                return AsString() == cmp.AsString();
            case Type::Data:
            case Type::Array:
            case Type::Object:
            case Type::Function:
            case Type::Pointer:
                return value_ == cmp.value_;
        }
    }
    
    bool Any::operator!= (const Any & cmp) const {
        return !(*this == cmp);
    }
    
    Any Any::Clone() const {
        switch (type_) {
            case Type::Null:
                return Any();
            case Type::Boolean:
                return Any(*AsBoolean());
            case Type::Number:
                return Any(*AsDouble());
            case Type::String:
                return Any(*AsString());
            case Type::Data:
                return Any(std::make_shared<Data>(**AsData()));
            case Type::Array: {
                std::vector<Any> array = Map(*AsArray(), [](const Any & x) -> Any {
                    return x.Clone();
                });
                return Any(array);
            }
            case Type::Object: {
                std::map<std::string, Any> map =
                MapToMap(*AsObject(), [](const std::string & key, const Any & x) {
                    return std::pair<std::string, Any>(key, x.Clone());
                });
                return Any(map);
            }
            case Type::Function:
                return Any(*AsFunction());
            case Type::Pointer:
                return Any(*AsPointer());
        }
    }
    
    Any Any::GetAt(int index) const {
        auto array = inner_array();
        if (array) {
            if (0 <= index && index < array->size()) {
                return (*array)[index];
            }
        }
        return nullptr;
    }
    void Any::SetAt(int index, const Any & value) {
        auto array = inner_array();
        if (!array) { Fatal("not array"); }
        if (index < 0) { Fatal("invalid index"); }
        if (array->size() <= index) {
            array->resize(index + 1);
        }
        (*array)[index] = value;
    }
    Any Any::GetAt(const std::string & key) const {
        auto dict = inner_object();
        if (dict) {
            if (HasKey(key)) {
                return (*dict)[key];
            }
        }
        return nullptr;
    }
    void Any::SetAt(const std::string & key, const Any & value) {
        auto dict = inner_object();
        if (!dict) { Fatal("not dictionary"); }
        (*dict)[key] = value;
    }
    void Any::RemoveAt(const std::string & key) {
        auto dict = inner_object();
        if (!dict) { Fatal("not dictionary"); }
        (*dict).erase(key);
    }
    bool Any::HasKey(const std::string & key) const {
        auto dict = inner_object();
        if (dict) {
            return nwr::HasKey(*dict, key);
        }
        return false;
    }
    
    std::shared_ptr<Json::Value> Any::ToJson() const {
        switch (type()) {
            case Type::Null:
                return std::make_shared<Json::Value>();
            case Type::Boolean:
                return std::make_shared<Json::Value>(*AsBoolean());
            case Type::Number:
                return std::make_shared<Json::Value>(*AsDouble());
            case Type::String:
                return std::make_shared<Json::Value>(*AsString());
            case Type::Data:
                return std::make_shared<Json::Value>(Format("<Data %s>", DataFormat(**AsData()).c_str()));
            case Type::Array: {
                auto json = std::make_shared<Json::Value>(Json::arrayValue);
                for (int i = 0; i < count(); i++) {
                    json->append(*GetAt(i).ToJson());
                }
                return json;
            }
            case Type::Object: {
                auto json = std::make_shared<Json::Value>(Json::objectValue);
                for (auto key : keys()) {
                    (*json)[key] = *GetAt(key).ToJson();
                }
                return json;
            }
            case Type::Function:
                return std::make_shared<Json::Value>(Format("<Function %p>", value_.get()));
            case Type::Pointer:
                return std::make_shared<Json::Value>(Format("<Pointer %p>", value_.get()));
        }
    }
    
    Any Any::FromJson(const Json::Value & json) {
        switch (json.type()) {
            case Json::nullValue:
                return Any(nullptr);
            case Json::intValue:
                return Any(json.asInt());
            case Json::uintValue:
                return Any(static_cast<double>(json.asUInt()));
            case Json::realValue:
                return Any(json.asDouble());
            case Json::stringValue:
                return Any(json.asString());
            case Json::booleanValue:
                return Any(json.asBool());
            case Json::arrayValue: {
                ArrayType array;
                for (int i = 0; i < json.size(); i++) {
                    array.push_back(FromJson(json[i]));
                }
                return Any(array);
            }
            case Json::objectValue: {
                ObjectType dict;
                for (auto key : json.getMemberNames()) {
                    dict[key] = FromJson(json.get(key, Json::Value()));
                }
                return Any(dict);
            }
        }
    }
    
    std::string Any::ToJsonString() const {
        return JsonFormat(*ToJson());
    }
    Any Any::FromJsonString(const std::string & str) {
        auto json = JsonParse(str);
        if (!json) { return nullptr; }
        return FromJson(*json);
    }
    
    std::shared_ptr<Any::ArrayType> Any::inner_array() const {
        if (type() == Type::Array) {
            return std::static_pointer_cast<ArrayType>(value_);
        } else {
            return nullptr;
        }
    }
    std::shared_ptr<Any::ObjectType> Any::inner_object() const {
        if (type() == Type::Object) {
            return std::static_pointer_cast<Any::ObjectType>(value_);
        } else {
            return nullptr;
        }
    }
}

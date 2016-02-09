//
//  any.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/10.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "any.h"

namespace nwr {
    Any::Any(): holder_(std::make_shared<NullHolder>()) {}
    Any::Any(const Any & copy): holder_(nullptr) {
        *this = copy;
    }
    Any::Any(Any && move): holder_(nullptr) {
        *this = move;
    }
    
    Any::Any(std::nullptr_t value): holder_(std::make_shared<NullHolder>()) {}
    Any::Any(bool value): holder_(std::make_shared<BooleanHolder>(value)) {}
    Any::Any(int value): holder_(std::make_shared<NumberHolder>(value)) {}
    Any::Any(double value): holder_(std::make_shared<NumberHolder>(value)) {}
    Any::Any(const std::string & value): holder_(std::make_shared<StringHolder>(value)) {}
    Any::Any(const Data & value): holder_(std::make_shared<DataHolder>(value)) {}
    Any::Any(const DataPtr & value): holder_(std::make_shared<DataHolder>(value)) {}
    Any::Any(const std::vector<Any> & value): holder_(std::make_shared<ArrayHolder>(value)) {}
    Any::Any(const std::shared_ptr<std::vector<Any>> & value): holder_(std::make_shared<ArrayHolder>(value)) {}
    Any::Any(const std::map<std::string, Any> & value): holder_(std::make_shared<DictionaryHolder>(value)) {}
    Any::Any(const std::shared_ptr<std::map<std::string, Any>> & value): holder_(std::make_shared<DictionaryHolder>(value)) {}
    
    Any::Type Any::type() const {
        return holder_->type;
    }
    Optional<bool> Any::AsBoolean() const {
        if (type() == Any::Type::Boolean) {
            return Some(std::static_pointer_cast<BooleanHolder>(holder_)->value);
        } else {
            return None();
        }
    }
    Optional<int> Any::AsInt() const {
        auto v = AsDouble();
        if (v) {
            return Some(static_cast<int>(*v));
        } else {
            return None();
        }
    }
    Optional<double> Any::AsDouble() const {
        if (type() == Any::Type::Number) {
            return Some(std::static_pointer_cast<NumberHolder>(holder_)->value);
        } else {
            return None();
        }
    }
    Optional<std::string> Any::AsString() const {
        if (type() == Any::Type::String) {
            return Some(std::static_pointer_cast<StringHolder>(holder_)->value);
        } else {
            return None();
        }
    }
    Optional<DataPtr> Any::AsData() const {
        if (type() == Any::Type::Data) {
            return Some(std::static_pointer_cast<DataHolder>(holder_)->value);
        } else {
            return None();
        }
    }
    Optional<std::shared_ptr<std::vector<Any>>> Any::AsArray() const {
        if (type() == Any::Type::Array) {
            return Some(std::static_pointer_cast<ArrayHolder>(holder_)->value);
        } else {
            return None();
        }
    }
    Optional<std::shared_ptr<std::map<std::string, Any>>> Any::AsDictionary() const {
        if (type() == Any::Type::Dictionary) {
            return Some(std::static_pointer_cast<DictionaryHolder>(holder_)->value);
        } else {
            return None();
        }
    }
    
    Any & Any::operator= (const Any & copy) {
        holder_ = std::shared_ptr<Any::Holder>(copy.holder_->Copy());
        return *this;
    }
    Any & Any::operator= (Any && move) {
        holder_ = move.holder_;
        move.holder_ = std::make_shared<NullHolder>();
        return *this;
    }
    Any & Any::operator= (std::nullptr_t value) { return *this = Any(value); }
    Any & Any::operator= (bool value) { return *this = Any(value); }
    Any & Any::operator= (int value) { return *this = Any(value); }
    Any & Any::operator= (double value) { return *this = Any(value); }
    Any & Any::operator= (const std::string & value) { return *this = Any(value); }
    Any & Any::operator= (const Data & value) { return *this = Any(value); }
    Any & Any::operator= (const DataPtr & value) { return *this = Any(value); }
    Any & Any::operator= (const std::vector<Any> & value) { return *this = Any(value); }
    Any & Any::operator= (const std::shared_ptr<std::vector<Any>> & value) { return *this = Any(value); }
    Any & Any::operator= (const std::map<std::string, Any> & value) { return *this = Any(value); }
    Any & Any::operator= (const std::shared_ptr<std::map<std::string, Any>> & value) { return *this = Any(value); }

}

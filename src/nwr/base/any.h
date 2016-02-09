//
//  any.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/10.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "data.h"
#include "optional.h"

namespace nwr {
    class Any {
    public:
        enum class Type {
            Null,
            Boolean,
            Number,
            String, // value
            Data, // ref
            Array, // ref
            Dictionary // ref
        };
        Any();
        Any(const Any & copy);
        Any(Any && move);
        
        explicit Any(std::nullptr_t value);
        explicit Any(bool value);
        explicit Any(int value);
        explicit Any(double value);
        explicit Any(const std::string & value);
        explicit Any(const Data & value);
        explicit Any(const DataPtr & value);
        explicit Any(const std::vector<Any> & value);
        explicit Any(const std::shared_ptr<std::vector<Any>> & value);
        explicit Any(const std::map<std::string, Any> & value);
        explicit Any(const std::shared_ptr<std::map<std::string, Any>> & value);
        
        Type type() const;
        
        Optional<bool> AsBoolean() const;
        Optional<int> AsInt() const;
        Optional<double> AsDouble() const;
        Optional<std::string> AsString() const;
        Optional<DataPtr> AsData() const;
        Optional<std::shared_ptr<std::vector<Any>>> AsArray() const;
        Optional<std::shared_ptr<std::map<std::string, Any>>> AsDictionary() const;
        
        Any & operator= (const Any & copy);
        Any & operator= (Any && move);
        
        Any & operator= (std::nullptr_t value);
        Any & operator= (bool value);
        Any & operator= (int value);
        Any & operator= (double value);
        Any & operator= (const std::string & value);
        Any & operator= (const Data & value);
        Any & operator= (const DataPtr & value);
        Any & operator= (const std::vector<Any> & value);
        Any & operator= (const std::shared_ptr<std::vector<Any>> & value);
        Any & operator= (const std::map<std::string, Any> & value);
        Any & operator= (const std::shared_ptr<std::map<std::string, Any>> & value);
    private:
        struct Holder {
            Holder(Type type): type(type) {}
            virtual ~Holder() {}
            virtual Holder * Copy() = 0;
            Type type;
        };
        struct NullHolder: public Holder {
            using ThisType = NullHolder;
            NullHolder(): Holder(Type::Null) {}
            virtual ThisType * Copy() { return new ThisType(); }
        };
        struct BooleanHolder: public Holder {
            using ThisType = BooleanHolder;
            BooleanHolder(bool value): Holder(Type::Boolean), value(value) {}
            virtual ThisType * Copy() { return new ThisType(value); }
            bool value;
        };
        struct NumberHolder: public Holder {
            using ThisType = NumberHolder;
            NumberHolder(int value): Holder(Type::Number), value(value) {}
            NumberHolder(double value): Holder(Type::Number), value(value) {}
            virtual ThisType * Copy() { return new ThisType(value); }
            double value;
        };
        struct StringHolder: public Holder {
            using ThisType = StringHolder;
            StringHolder(const std::string & value): Holder(Type::String), value(value) {}
            virtual ThisType * Copy() { return new ThisType(value); }
            std::string value;
        };
        struct DataHolder: public Holder {
            using ThisType = DataHolder;
            DataHolder(const Data & value):
            DataHolder(std::make_shared<Data>(value)){}
            
            DataHolder(const DataPtr & value): Holder(Type::Data), value(value) {}
            
            virtual ThisType * Copy() { return new ThisType(value); }
            DataPtr value;
        };
        struct ArrayHolder: public Holder {
            using ThisType = ArrayHolder;
            ArrayHolder(const std::vector<Any> & value):
            ArrayHolder(std::make_shared<std::vector<Any>>(value)){}
            
            ArrayHolder(std::shared_ptr<std::vector<Any>> value):
            Holder(Type::Array), value(value) {}
            
            virtual ThisType * Copy() { return new ThisType(value); }
            std::shared_ptr<std::vector<Any>> value;
        };
        struct DictionaryHolder: public Holder {
            using ThisType = DictionaryHolder;
            DictionaryHolder(const std::map<std::string, Any> & value):
            DictionaryHolder(std::make_shared<std::map<std::string, Any>>(value)){}
            
            DictionaryHolder(std::shared_ptr<std::map<std::string, Any>> value):
            Holder(Type::Dictionary), value(value) {}
            
            virtual ThisType * Copy() { return new ThisType(value); }
            std::shared_ptr<std::map<std::string, Any>> value;
        };
        std::shared_ptr<Holder> holder_;
    };
}
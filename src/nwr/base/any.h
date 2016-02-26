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
#include "func.h"
#include "any_func_forward.h"

namespace Json {
    class Value;
}

namespace nwr {
    class Any {
    public:
        using ArrayType = std::vector<Any>;
        using ObjectType = std::map<std::string, Any>;
        using PointerType = std::shared_ptr<void>;
        
        enum class Type {
            //  value types
            Null,
            Boolean,
            Number,
            String,
            //  ref types
            Data,
            Array,
            Object,
            Function,
            Pointer
        };
        Any();
        Any(const Any & copy);
        Any(Any && move);
        
        Any(std::nullptr_t value);
        explicit Any(bool value);
        explicit Any(int value);
        explicit Any(double value);
        explicit Any(const char * value);
        explicit Any(const std::string & value);
        explicit Any(const Data & value);
        explicit Any(const DataPtr & value);
        explicit Any(const ArrayType & value);
        explicit Any(const ObjectType & value);
        Any(const AnyFuncPtr & value);
        explicit Any(const PointerType & value);
        template <typename T> explicit Any(const Optional<T> & value):
        Any(value ? *value : nullptr)
        {}
        
        Type type() const;
        int count() const;
        std::vector<std::string> keys() const;
        
        explicit operator bool() const;
        
        Optional<bool> AsBoolean() const;
        Optional<int> AsInt() const;
        Optional<double> AsDouble() const;
        Optional<std::string> AsString() const;
        Optional<DataPtr> AsData() const;
        Optional<ArrayType> AsArray() const;
        Optional<ObjectType> AsObject() const;
        Optional<AnyFuncPtr> AsFunction() const;
        Optional<PointerType> AsPointer() const;
        
        Any & operator= (const Any & copy);
        Any & operator= (Any && move);
        
        bool operator== (const Any & cmp) const;
        bool operator!= (const Any & cmp) const;
        
        // can not clone Pointer
        Any Clone() const;

        Any GetAt(int index) const;
        void SetAt(int index, const Any & value);
        
        Any GetAt(const std::string & key) const;
        void SetAt(const std::string & key, const Any & value);
        void RemoveAt(const std::string & key);
        bool HasKey(const std::string & key) const;
        
        std::shared_ptr<Json::Value> ToJson() const;
        static Any FromJson(const Json::Value & json);
        
        std::string ToJsonString() const;
        static Any FromJsonString(const std::string & str);
    private:
        std::shared_ptr<ArrayType> inner_array() const;
        std::shared_ptr<ObjectType> inner_object() const;
        
        Type type_;
        std::shared_ptr<void> value_;
    };

}
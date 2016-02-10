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

namespace Json {
    class Value;
}

namespace nwr {
    class Any {
    public:
        enum class Type {
            //  value types
            Null,
            Boolean,
            Number,
            String,
            //  ref types
            Data,
            Array,
            Dictionary
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
        explicit Any(const std::vector<Any> & value);
        explicit Any(const std::map<std::string, Any> & value);
        
        Type type() const;
        int count() const;
        std::vector<std::string> keys() const;
        
        Optional<bool> AsBoolean() const;
        Optional<int> AsInt() const;
        Optional<double> AsDouble() const;
        Optional<std::string> AsString() const;
        Optional<DataPtr> AsData() const;
        Optional<std::vector<Any>> AsArray() const;
        Optional<std::map<std::string, Any>> AsDictionary() const;
        
        Any & operator= (const Any & copy);
        Any & operator= (Any && move);
                
        Any GetAt(int index) const;
        void SetAt(int index, const Any & value);
        
        Any GetAt(const std::string & key) const;
        void SetAt(const std::string & key, const Any & value);
        
        static Any FromJson(const Json::Value & json);
        std::shared_ptr<Json::Value> ToJson() const;
    private:
        std::shared_ptr<std::vector<Any>> inner_array() const;
        std::shared_ptr<std::map<std::string, Any>> inner_dictionary() const;
        
        Type type_;
        std::shared_ptr<void> value_;
    };
}
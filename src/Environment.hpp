#ifndef BYSHARP_ENVIRONMENT_HPP
#define BYSHARP_ENVIRONMENT_HPP

#include "Common.hpp"
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

class FnDeclStmt;

class Environment : public std::enable_shared_from_this<Environment> {
public:
    std::shared_ptr<Environment> enclosing;

    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : enclosing(std::move(parent)) {}

    void define(const std::string& name, const Value& value, DataType type) {
        Value val = convertValue(value, type);
        values[name] = val;
        types[name] = type;
    }

    void assign(const std::string& name, const Value& value) {
        auto it = values.find(name);
        if (it != values.end()) {
            DataType targetType = types[name];
            values[name] = convertValue(value, targetType);
            return;
        }
        if (enclosing) {
            enclosing->assign(name, value);
            return;
        }
        throw RuntimeError("Runtime Error: Undefined variable '" + name + "'");
    }

    Value get(const std::string& name) {
        auto it = values.find(name);
        if (it != values.end()) {
            return it->second;
        }
        if (enclosing) {
            return enclosing->get(name);
        }
        throw RuntimeError("Runtime Error: Undefined variable '" + name + "'");
    }

    bool has(const std::string& name) const {
        if (values.find(name) != values.end()) {
            return true;
        }
        if (enclosing) {
            return enclosing->has(name);
        }
        return false;
    }

    void defineFunction(const std::string& name, FnDeclStmt* fn) {
        functions[name] = fn;
    }

    FnDeclStmt* getFunction(const std::string& name) {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second;
        }
        if (enclosing) {
            return enclosing->getFunction(name);
        }
        return nullptr;
    }

    void defineNative(const std::string& name, std::function<Value(const std::vector<Value>&)> fn) {
        natives[name] = fn;
    }

    bool hasNative(const std::string& name) const {
        if (natives.find(name) != natives.end()) {
            return true;
        }
        if (enclosing) {
            return enclosing->hasNative(name);
        }
        return false;
    }

    Value callNative(const std::string& name, const std::vector<Value>& args) {
        auto it = natives.find(name);
        if (it != natives.end()) {
            return it->second(args);
        }
        if (enclosing) {
            return enclosing->callNative(name, args);
        }
        throw RuntimeError("Runtime Error: Undefined native function '" + name + "'");
    }

private:
    std::unordered_map<std::string, Value> values;
    std::unordered_map<std::string, DataType> types;
    std::unordered_map<std::string, FnDeclStmt*> functions;
    std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>> natives;

    Value convertValue(const Value& val, DataType targetType) {
        if (val.type == targetType) {
            return val;
        }
        if (targetType == DataType::INT) {
            if (val.type == DataType::FLOAT) return Value::makeInt(static_cast<int64_t>(val.floatVal));
            if (val.type == DataType::BYTE) return Value::makeInt(static_cast<int64_t>(val.byteVal));
            if (val.type == DataType::BOOL) return Value::makeInt(val.boolVal ? 1 : 0);
        } else if (targetType == DataType::FLOAT) {
            if (val.type == DataType::INT) return Value::makeFloat(static_cast<double>(val.intVal));
            if (val.type == DataType::BYTE) return Value::makeFloat(static_cast<double>(val.byteVal));
        } else if (targetType == DataType::BYTE) {
            if (val.type == DataType::INT) return Value::makeByte(static_cast<uint8_t>(val.intVal & 0xFF));
            if (val.type == DataType::FLOAT) return Value::makeByte(static_cast<uint8_t>(static_cast<int64_t>(val.floatVal) & 0xFF));
        }
        if (val.type == DataType::VOID && targetType != DataType::VOID) {
            switch (targetType) {
                case DataType::INT: return Value::makeInt(0);
                case DataType::FLOAT: return Value::makeFloat(0.0);
                case DataType::BOOL: return Value::makeBool(false);
                case DataType::STR: return Value::makeStr("");
                case DataType::BYTE: return Value::makeByte(0);
                default: break;
            }
        }
        throw RuntimeError("Runtime Error: Cannot convert type " + dataTypeToString(val.type) + " to " + dataTypeToString(targetType));
    }
};

#endif

#ifndef BYSHARP_COMMON_HPP
#define BYSHARP_COMMON_HPP

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <iomanip>
#include <cmath>

enum class DataType {
    INT,
    FLOAT,
    BOOL,
    STR,
    BYTE,
    VOID
};

inline std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT:
            return "int";
        case DataType::FLOAT:
            return "float";
        case DataType::BOOL:
            return "bool";
        case DataType::STR:
            return "str";
        case DataType::BYTE:
            return "byte";
        case DataType::VOID:
            return "void";
        default:
            return "unknown";
    }
}

enum class TokenType {
    TOKEN_EOF,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STR_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IDENTIFIER,
    TOKEN_KW_INT,
    TOKEN_KW_FLOAT,
    TOKEN_KW_BOOL,
    TOKEN_KW_STR,
    TOKEN_KW_BYTE,
    TOKEN_KW_VOID,
    TOKEN_KW_EXIT,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_AT,
    TOKEN_ASSIGN,
    TOKEN_ARROW,
    TOKEN_QUESTION,
    TOKEN_TILDE,
    TOKEN_LANGLE,
    TOKEN_RANGLE,
    TOKEN_SHR,
    TOKEN_INPUT,
    TOKEN_HASH,
    TOKEN_CARET,
    TOKEN_UNDERSCORE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT
};

struct Value {
    DataType type = DataType::VOID;
    int64_t intVal = 0;
    double floatVal = 0.0;
    bool boolVal = false;
    std::string strVal = "";
    uint8_t byteVal = 0;

    static Value makeInt(int64_t v) {
        Value val;
        val.type = DataType::INT;
        val.intVal = v;
        return val;
    }

    static Value makeFloat(double v) {
        Value val;
        val.type = DataType::FLOAT;
        val.floatVal = v;
        return val;
    }

    static Value makeBool(bool v) {
        Value val;
        val.type = DataType::BOOL;
        val.boolVal = v;
        return val;
    }

    static Value makeStr(const std::string& v) {
        Value val;
        val.type = DataType::STR;
        val.strVal = v;
        return val;
    }

    static Value makeByte(uint8_t v) {
        Value val;
        val.type = DataType::BYTE;
        val.byteVal = v;
        return val;
    }

    static Value makeVoid() {
        Value val;
        val.type = DataType::VOID;
        return val;
    }

    std::string toString() const {
        switch (type) {
            case DataType::INT:
                return std::to_string(intVal);
            case DataType::FLOAT: {
                std::ostringstream ss;
                ss << floatVal;
                return ss.str();
            }
            case DataType::BOOL:
                return boolVal ? "true" : "false";
            case DataType::STR:
                return strVal;
            case DataType::BYTE:
                return std::to_string(static_cast<int>(byteVal));
            case DataType::VOID:
            default:
                return "";
        }
    }

    bool isTruthy() const {
        switch (type) {
            case DataType::BOOL:
                return boolVal;
            case DataType::INT:
                return intVal != 0;
            case DataType::FLOAT:
                return floatVal != 0.0;
            case DataType::BYTE:
                return byteVal != 0;
            case DataType::STR:
                return !strVal.empty();
            default:
                return false;
        }
    }
};

struct Token {
    TokenType type;
    std::string lexeme;
    Value literal;
    int line;
    int column;
};

class SyntaxError : public std::runtime_error {
public:
    int line;
    int column;
    SyntaxError(const std::string& msg, int l = 0, int c = 0)
        : std::runtime_error(msg), line(l), column(c) {}
};

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string& msg)
        : std::runtime_error(msg) {}
};

class ReturnSignal {
public:
    Value value;
    ReturnSignal(Value val) : value(val) {}
};

class BreakSignal {};
class ContinueSignal {};

#endif

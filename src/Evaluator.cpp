#include "Evaluator.hpp"
#include <chrono>
#include <thread>
#include <fstream>
#include <random>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#ifdef VOID
#undef VOID
#endif
#ifdef byte
#undef byte
#endif
using socket_t = SOCKET;
#define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
#define CLOSE_SOCKET(s) closesocket(s)
#define popen _popen
#define pclose _pclose
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
using socket_t = int;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define IS_INVALID_SOCKET(s) ((s) < 0)
#define CLOSE_SOCKET(s) close(s)
#endif

namespace fs = std::filesystem;

static std::unordered_map<int64_t, socket_t> globalActiveSockets;
static int64_t globalNextSocketId = 1;

static bool initNetwork() {
#ifdef _WIN32
    static bool initialized = false;
    if (!initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }
        initialized = true;
    }
#endif
    return true;
}

static std::string execCommand(const std::string& cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    std::string res;
    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) {
        res += buf;
    }
    pclose(pipe);
    return res;
}

static std::string nativeHttpGet(const std::string& url) {
    if (url.rfind("http:" "/" "/", 0) != 0) return "";
    std::string rest = url.substr(7);
    std::string host, path = "/", portStr = "80";
    size_t slashPos = rest.find('/');
    if (slashPos != std::string::npos) {
        host = rest.substr(0, slashPos);
        path = rest.substr(slashPos);
    } else {
        host = rest;
    }
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        portStr = host.substr(colonPos + 1);
        host = host.substr(0, colonPos);
    }
    initNetwork();
    struct addrinfo hints{}, *resInfo = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &resInfo) != 0 || resInfo == nullptr) {
        return "";
    }
    socket_t s = socket(resInfo->ai_family, resInfo->ai_socktype, resInfo->ai_protocol);
    if (IS_INVALID_SOCKET(s)) {
        freeaddrinfo(resInfo);
        return "";
    }
    if (connect(s, resInfo->ai_addr, (int)resInfo->ai_addrlen) == SOCKET_ERROR) {
        CLOSE_SOCKET(s);
        freeaddrinfo(resInfo);
        return "";
    }
    freeaddrinfo(resInfo);
    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: BYSharp/1.0\r\nAccept: text/html, *" "/" "*\r\nConnection: close\r\n\r\n";
    send(s, req.c_str(), (int)req.length(), 0);
    std::string response;
    char buf[4096];
    int n = 0;
    while ((n = recv(s, buf, sizeof(buf), 0)) > 0) {
        response.append(buf, n);
    }
    CLOSE_SOCKET(s);
    size_t bodyPos = response.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        return response.substr(bodyPos + 4);
    }
    return response;
}

static std::string nativeHttpPost(const std::string& url, const std::string& data) {
    if (url.rfind("http:" "/" "/", 0) != 0) return "";
    std::string rest = url.substr(7);
    std::string host, path = "/", portStr = "80";
    size_t slashPos = rest.find('/');
    if (slashPos != std::string::npos) {
        host = rest.substr(0, slashPos);
        path = rest.substr(slashPos);
    } else {
        host = rest;
    }
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        portStr = host.substr(colonPos + 1);
        host = host.substr(0, colonPos);
    }
    initNetwork();
    struct addrinfo hints{}, *resInfo = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &resInfo) != 0 || resInfo == nullptr) {
        return "";
    }
    socket_t s = socket(resInfo->ai_family, resInfo->ai_socktype, resInfo->ai_protocol);
    if (IS_INVALID_SOCKET(s)) {
        freeaddrinfo(resInfo);
        return "";
    }
    if (connect(s, resInfo->ai_addr, (int)resInfo->ai_addrlen) == SOCKET_ERROR) {
        CLOSE_SOCKET(s);
        freeaddrinfo(resInfo);
        return "";
    }
    freeaddrinfo(resInfo);
    std::string req = "POST " + path + " HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: BYSharp/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: " + std::to_string(data.length()) + "\r\nConnection: close\r\n\r\n" + data;
    send(s, req.c_str(), (int)req.length(), 0);
    std::string response;
    char buf[4096];
    int n = 0;
    while ((n = recv(s, buf, sizeof(buf), 0)) > 0) {
        response.append(buf, n);
    }
    CLOSE_SOCKET(s);
    size_t bodyPos = response.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        return response.substr(bodyPos + 4);
    }
    return response;
}

Evaluator::Evaluator(std::shared_ptr<Environment> env) {
    if (env) {
        environment = env;
    } else {
        environment = std::make_shared<Environment>();
    }
    registerNatives();
}

bool Evaluator::isNumeric(const Value& val) const {
    return val.type == DataType::INT || val.type == DataType::FLOAT || val.type == DataType::BYTE;
}

double Evaluator::toDouble(const Value& val) const {
    if (val.type == DataType::FLOAT) return val.floatVal;
    if (val.type == DataType::INT) return static_cast<double>(val.intVal);
    if (val.type == DataType::BYTE) return static_cast<double>(val.byteVal);
    return 0.0;
}

int64_t Evaluator::toInt64(const Value& val) const {
    if (val.type == DataType::INT) return val.intVal;
    if (val.type == DataType::BYTE) return static_cast<int64_t>(val.byteVal);
    if (val.type == DataType::FLOAT) return static_cast<int64_t>(val.floatVal);
    return 0;
}

void Evaluator::registerNatives() {
    environment->defineNative("len", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) {
            throw RuntimeError("Runtime Error: len() expects 1 str argument");
        }
        return Value::makeInt(static_cast<int64_t>(args[0].strVal.length()));
    });

    environment->defineNative("to_int", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) {
            throw RuntimeError("Runtime Error: to_int() expects 1 argument");
        }
        if (args[0].type == DataType::STR) {
            try {
                return Value::makeInt(std::stoll(args[0].strVal));
            } catch (...) {
                throw RuntimeError("Runtime Error: Invalid string format for to_int");
            }
        }
        if (args[0].type == DataType::BOOL) {
            return Value::makeInt(args[0].boolVal ? 1 : 0);
        }
        return Value::makeInt(toInt64(args[0]));
    });

    environment->defineNative("to_float", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) {
            throw RuntimeError("Runtime Error: to_float() expects 1 argument");
        }
        if (args[0].type == DataType::STR) {
            try {
                return Value::makeFloat(std::stod(args[0].strVal));
            } catch (...) {
                throw RuntimeError("Runtime Error: Invalid string format for to_float");
            }
        }
        return Value::makeFloat(toDouble(args[0]));
    });

    environment->defineNative("to_str", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) {
            throw RuntimeError("Runtime Error: to_str() expects 1 argument");
        }
        return Value::makeStr(args[0].toString());
    });

    environment->defineNative("to_byte", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) {
            throw RuntimeError("Runtime Error: to_byte() expects 1 argument");
        }
        return Value::makeByte(static_cast<uint8_t>(toInt64(args[0]) & 0xFF));
    });

    environment->defineNative("ord", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) {
            throw RuntimeError("Runtime Error: ord() expects 1 str argument");
        }
        if (args[0].strVal.empty()) {
            return Value::makeInt(0);
        }
        return Value::makeInt(static_cast<int64_t>(static_cast<unsigned char>(args[0].strVal[0])));
    });

    environment->defineNative("chr", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) {
            throw RuntimeError("Runtime Error: chr() expects 1 integer argument");
        }
        char c = static_cast<char>(toInt64(args[0]) & 0xFF);
        return Value::makeStr(std::string(1, c));
    });

    environment->defineNative("abs", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: abs() expects 1 argument");
        if (args[0].type == DataType::FLOAT) return Value::makeFloat(std::fabs(args[0].floatVal));
        if (args[0].type == DataType::INT) return Value::makeInt(std::abs(args[0].intVal));
        if (args[0].type == DataType::BYTE) return Value::makeInt(static_cast<int64_t>(args[0].byteVal));
        throw RuntimeError("Runtime Error: abs() requires numeric argument");
    });

    environment->defineNative("min", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw RuntimeError("Runtime Error: min() expects 2 arguments");
        if (!isNumeric(args[0]) || !isNumeric(args[1])) throw RuntimeError("Runtime Error: min() requires numeric arguments");
        if (args[0].type == DataType::FLOAT || args[1].type == DataType::FLOAT) {
            return Value::makeFloat(std::min(toDouble(args[0]), toDouble(args[1])));
        }
        return Value::makeInt(std::min(toInt64(args[0]), toInt64(args[1])));
    });

    environment->defineNative("max", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw RuntimeError("Runtime Error: max() expects 2 arguments");
        if (!isNumeric(args[0]) || !isNumeric(args[1])) throw RuntimeError("Runtime Error: max() requires numeric arguments");
        if (args[0].type == DataType::FLOAT || args[1].type == DataType::FLOAT) {
            return Value::makeFloat(std::max(toDouble(args[0]), toDouble(args[1])));
        }
        return Value::makeInt(std::max(toInt64(args[0]), toInt64(args[1])));
    });

    environment->defineNative("pow", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw RuntimeError("Runtime Error: pow() expects 2 arguments");
        if (!isNumeric(args[0]) || !isNumeric(args[1])) throw RuntimeError("Runtime Error: pow() requires numeric arguments");
        return Value::makeFloat(std::pow(toDouble(args[0]), toDouble(args[1])));
    });

    environment->defineNative("sqrt", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: sqrt() expects 1 argument");
        double v = toDouble(args[0]);
        if (v < 0.0) throw RuntimeError("Runtime Error: sqrt() of negative number");
        return Value::makeFloat(std::sqrt(v));
    });

    environment->defineNative("floor", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: floor() expects 1 argument");
        return Value::makeFloat(std::floor(toDouble(args[0])));
    });

    environment->defineNative("ceil", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: ceil() expects 1 argument");
        return Value::makeFloat(std::ceil(toDouble(args[0])));
    });

    environment->defineNative("round", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: round() expects 1 argument");
        return Value::makeInt(static_cast<int64_t>(std::round(toDouble(args[0]))));
    });

    environment->defineNative("sin", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: sin() expects 1 argument");
        return Value::makeFloat(std::sin(toDouble(args[0])));
    });

    environment->defineNative("cos", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: cos() expects 1 argument");
        return Value::makeFloat(std::cos(toDouble(args[0])));
    });

    environment->defineNative("tan", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: tan() expects 1 argument");
        return Value::makeFloat(std::tan(toDouble(args[0])));
    });

    environment->defineNative("log", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: log() expects 1 argument");
        double v = toDouble(args[0]);
        if (v <= 0.0) throw RuntimeError("Runtime Error: log() argument must be positive");
        return Value::makeFloat(std::log(v));
    });

    environment->defineNative("log10", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: log10() expects 1 argument");
        double v = toDouble(args[0]);
        if (v <= 0.0) throw RuntimeError("Runtime Error: log10() argument must be positive");
        return Value::makeFloat(std::log10(v));
    });

    environment->defineNative("random", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw RuntimeError("Runtime Error: random() expects 2 arguments (min, max)");
        int64_t minVal = toInt64(args[0]);
        int64_t maxVal = toInt64(args[1]);
        if (minVal > maxVal) std::swap(minVal, maxVal);
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        std::uniform_int_distribution<int64_t> dis(minVal, maxVal);
        return Value::makeInt(dis(gen));
    });

    environment->defineNative("substr", [this](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || args.size() > 3) throw RuntimeError("Runtime Error: substr() expects 2 or 3 arguments (str, start, [len])");
        if (args[0].type != DataType::STR) throw RuntimeError("Runtime Error: substr() first argument must be str");
        int64_t start = toInt64(args[1]);
        const std::string& s = args[0].strVal;
        if (start < 0 || start >= static_cast<int64_t>(s.length())) return Value::makeStr("");
        if (args.size() == 2) {
            return Value::makeStr(s.substr(start));
        }
        int64_t len = toInt64(args[2]);
        if (len < 0) return Value::makeStr("");
        return Value::makeStr(s.substr(start, len));
    });

    environment->defineNative("find", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw RuntimeError("Runtime Error: find() expects 2 str arguments");
        if (args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: find() requires str arguments");
        size_t pos = args[0].strVal.find(args[1].strVal);
        if (pos == std::string::npos) return Value::makeInt(-1);
        return Value::makeInt(static_cast<int64_t>(pos));
    });

    environment->defineNative("contains", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw RuntimeError("Runtime Error: contains() expects 2 str arguments");
        if (args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: contains() requires str arguments");
        bool has = args[0].strVal.find(args[1].strVal) != std::string::npos;
        return Value::makeBool(has);
    });

    environment->defineNative("replace", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 3) throw RuntimeError("Runtime Error: replace() expects 3 str arguments (source, target, replacement)");
        if (args[0].type != DataType::STR || args[1].type != DataType::STR || args[2].type != DataType::STR) throw RuntimeError("Runtime Error: replace() requires str arguments");
        std::string s = args[0].strVal;
        const std::string& from = args[1].strVal;
        const std::string& to = args[2].strVal;
        if (from.empty()) return Value::makeStr(s);
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.length(), to);
            pos += to.length();
        }
        return Value::makeStr(s);
    });

    environment->defineNative("upper", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: upper() expects 1 str argument");
        std::string s = args[0].strVal;
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return Value::makeStr(s);
    });

    environment->defineNative("lower", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: lower() expects 1 str argument");
        std::string s = args[0].strVal;
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return Value::makeStr(s);
    });

    environment->defineNative("trim", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: trim() expects 1 str argument");
        const std::string& s = args[0].strVal;
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return Value::makeStr("");
        size_t last = s.find_last_not_of(" \t\r\n");
        return Value::makeStr(s.substr(first, last - first + 1));
    });

    environment->defineNative("to_bin", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: to_bin() expects 1 str argument");
        std::string res = "";
        for (size_t i = 0; i < args[0].strVal.size(); ++i) {
            if (i > 0) res += " ";
            uint8_t byteVal = static_cast<uint8_t>(args[0].strVal[i]);
            for (int b = 7; b >= 0; --b) {
                res += ((byteVal >> b) & 1) ? '1' : '0';
            }
        }
        return Value::makeStr(res);
    });

    environment->defineNative("from_bin", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: from_bin() expects 1 str argument");
        std::string bits = "";
        for (char c : args[0].strVal) {
            if (c == '0' || c == '1') bits.push_back(c);
        }
        if (bits.length() % 8 != 0) throw RuntimeError("Runtime Error: from_bin() bits count must be a multiple of 8");
        std::string res = "";
        for (size_t i = 0; i < bits.length(); i += 8) {
            uint8_t b = 0;
            for (size_t j = 0; j < 8; ++j) {
                b = static_cast<uint8_t>((b << 1) | (bits[i + j] - '0'));
            }
            res.push_back(static_cast<char>(b));
        }
        return Value::makeStr(res);
    });

    environment->defineNative("time", [](const std::vector<Value>&) -> Value {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return Value::makeInt(static_cast<int64_t>(ms));
    });

    environment->defineNative("clock", [](const std::vector<Value>&) -> Value {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return Value::makeInt(static_cast<int64_t>(ms));
    });

    environment->defineNative("sleep", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw RuntimeError("Runtime Error: sleep() expects 1 integer argument (ms)");
        int64_t ms = toInt64(args[0]);
        if (ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        return Value::makeVoid();
    });

    environment->defineNative("file_exists", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_exists() expects 1 str path argument");
        std::ifstream f(args[0].strVal);
        return Value::makeBool(f.good());
    });

    environment->defineNative("file_read", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_read() expects 1 str path argument");
        std::ifstream f(args[0].strVal);
        if (!f.is_open()) throw RuntimeError("Runtime Error: file_read() could not open file: " + args[0].strVal);
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return Value::makeStr(content);
    });

    environment->defineNative("file_write", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: file_write() expects 2 str arguments (path, content)");
        std::ofstream f(args[0].strVal);
        if (!f.is_open()) throw RuntimeError("Runtime Error: file_write() could not create file: " + args[0].strVal);
        f << args[1].strVal;
        return Value::makeBool(true);
    });

    environment->defineNative("file_append", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: file_append() expects 2 str arguments (path, content)");
        std::ofstream f(args[0].strVal, std::ios::app);
        if (!f.is_open()) throw RuntimeError("Runtime Error: file_append() could not open file: " + args[0].strVal);
        f << args[1].strVal;
        return Value::makeBool(true);
    });

    environment->defineNative("file_delete", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_delete() expects 1 str argument (path)");
        try {
            return Value::makeBool(fs::remove(args[0].strVal));
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("file_remove", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_remove() expects 1 str argument (path)");
        try {
            return Value::makeBool(fs::remove(args[0].strVal));
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("file_size", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_size() expects 1 str argument (path)");
        try {
            if (fs::exists(args[0].strVal) && fs::is_regular_file(args[0].strVal)) {
                return Value::makeInt(static_cast<int64_t>(fs::file_size(args[0].strVal)));
            }
        } catch (...) {}
        return Value::makeInt(-1);
    });

    environment->defineNative("file_copy", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: file_copy() expects 2 str arguments (src, dst)");
        try {
            fs::copy_file(args[0].strVal, args[1].strVal, fs::copy_options::overwrite_existing);
            return Value::makeBool(true);
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("file_rename", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: file_rename() expects 2 str arguments (old_path, new_path)");
        try {
            fs::rename(args[0].strVal, args[1].strVal);
            return Value::makeBool(true);
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("dir_create", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: dir_create() expects 1 str argument (path)");
        try {
            return Value::makeBool(fs::create_directories(args[0].strVal));
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("dir_exists", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: dir_exists() expects 1 str argument (path)");
        try {
            return Value::makeBool(fs::exists(args[0].strVal) && fs::is_directory(args[0].strVal));
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("dir_remove", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: dir_remove() expects 1 str argument (path)");
        try {
            return Value::makeBool(fs::remove_all(args[0].strVal) > 0);
        } catch (...) {
            return Value::makeBool(false);
        }
    });

    environment->defineNative("dir_list", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: dir_list() expects 1 str argument (path)");
        std::string res = "";
        try {
            if (fs::exists(args[0].strVal) && fs::is_directory(args[0].strVal)) {
                for (const auto& entry : fs::directory_iterator(args[0].strVal)) {
                    if (!res.empty()) res += "\n";
                    res += entry.path().filename().string();
                }
            }
        } catch (...) {}
        return Value::makeStr(res);
    });

    environment->defineNative("console_clear", [](const std::vector<Value>&) -> Value {
        std::cout << "\033[2J\033[H";
        std::cout.flush();
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        return Value::makeVoid();
    });

    environment->defineNative("io_flush", [](const std::vector<Value>&) -> Value {
        std::cout.flush();
        return Value::makeVoid();
    });

    environment->defineNative("io_read_line", [](const std::vector<Value>&) -> Value {
        std::string line;
        if (std::getline(std::cin, line)) {
            return Value::makeStr(line);
        }
        return Value::makeStr("");
    });

    environment->defineNative("sys_exec", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: sys_exec() expects 1 str argument (command)");
        return Value::makeStr(execCommand(args[0].strVal));
    });

    environment->defineNative("http_get", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: http_get() expects 1 str argument (url)");
        std::string url = args[0].strVal;
        std::string cmd = "curl -s -L \"" + url + "\"";
        std::string res = execCommand(cmd);
        if (!res.empty()) {
            return Value::makeStr(res);
        }
        std::string nat = nativeHttpGet(url);
        return Value::makeStr(nat);
    });

    environment->defineNative("http_post", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) throw RuntimeError("Runtime Error: http_post() expects 2 str arguments (url, data)");
        std::string url = args[0].strVal;
        std::string data = args[1].strVal;
        std::string safeData = data;
        for (size_t i = 0; i < safeData.length(); ++i) {
            if (safeData[i] == '"') {
                safeData.insert(i, "\\");
                i++;
            }
        }
        std::string cmd = "curl -s -L -X POST --data-raw \"" + safeData + "\" \"" + url + "\"";
        std::string res = execCommand(cmd);
        if (!res.empty()) {
            return Value::makeStr(res);
        }
        std::string nat = nativeHttpPost(url, data);
        return Value::makeStr(nat);
    });

    environment->defineNative("url_encode", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: url_encode() expects 1 str argument");
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex << std::uppercase;
        for (char c : args[0].strVal) {
            if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::setw(2) << ((int)(unsigned char)c);
            }
        }
        return Value::makeStr(escaped.str());
    });

    environment->defineNative("url_decode", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: url_decode() expects 1 str argument");
        std::string str = args[0].strVal;
        std::string res = "";
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                std::string hex = str.substr(i + 1, 2);
                char ch = (char)std::strtol(hex.c_str(), nullptr, 16);
                res += ch;
                i += 2;
            } else if (str[i] == '+') {
                res += ' ';
            } else {
                res += str[i];
            }
        }
        return Value::makeStr(res);
    });

    environment->defineNative("net_ip_lookup", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: net_ip_lookup() expects 1 str argument (host)");
        initNetwork();
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(args[0].strVal.c_str(), nullptr, &hints, &res) == 0 && res != nullptr) {
            char ipStr[INET_ADDRSTRLEN];
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN);
            freeaddrinfo(res);
            return Value::makeStr(std::string(ipStr));
        }
        if (res) freeaddrinfo(res);
        return Value::makeStr("");
    });

    environment->defineNative("net_connect", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || !isNumeric(args[1])) {
            throw RuntimeError("Runtime Error: net_connect() expects 2 arguments (host: str, port: int)");
        }
        initNetwork();
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        std::string portStr = std::to_string(toInt64(args[1]));
        if (getaddrinfo(args[0].strVal.c_str(), portStr.c_str(), &hints, &res) != 0 || res == nullptr) {
            return Value::makeInt(-1);
        }
        socket_t sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (IS_INVALID_SOCKET(sock)) {
            freeaddrinfo(res);
            return Value::makeInt(-1);
        }
        if (connect(sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
            CLOSE_SOCKET(sock);
            freeaddrinfo(res);
            return Value::makeInt(-1);
        }
        freeaddrinfo(res);
        int64_t id = globalNextSocketId++;
        globalActiveSockets[id] = sock;
        return Value::makeInt(id);
    });

    environment->defineNative("net_send", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || !isNumeric(args[0]) || args[1].type != DataType::STR) {
            throw RuntimeError("Runtime Error: net_send() expects 2 arguments (socket_id: int, data: str)");
        }
        int64_t id = toInt64(args[0]);
        auto it = globalActiveSockets.find(id);
        if (it == globalActiveSockets.end()) return Value::makeInt(-1);
        int sent = send(it->second, args[1].strVal.c_str(), (int)args[1].strVal.length(), 0);
        return Value::makeInt(sent);
    });

    environment->defineNative("net_recv", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || !isNumeric(args[0]) || !isNumeric(args[1])) {
            throw RuntimeError("Runtime Error: net_recv() expects 2 arguments (socket_id: int, max_bytes: int)");
        }
        int64_t id = toInt64(args[0]);
        auto it = globalActiveSockets.find(id);
        if (it == globalActiveSockets.end()) return Value::makeStr("");
        int64_t maxLen = toInt64(args[1]);
        if (maxLen <= 0) maxLen = 4096;
        std::vector<char> buf(static_cast<size_t>(maxLen));
        int bytesRead = recv(it->second, buf.data(), (int)maxLen, 0);
        if (bytesRead <= 0) return Value::makeStr("");
        return Value::makeStr(std::string(buf.data(), static_cast<size_t>(bytesRead)));
    });

    environment->defineNative("net_close", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || !isNumeric(args[0])) {
            throw RuntimeError("Runtime Error: net_close() expects 1 int argument (socket_id)");
        }
        int64_t id = toInt64(args[0]);
        auto it = globalActiveSockets.find(id);
        if (it == globalActiveSockets.end()) return Value::makeBool(false);
        CLOSE_SOCKET(it->second);
        globalActiveSockets.erase(it);
        return Value::makeBool(true);
    });

    environment->defineNative("net_listen", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || !isNumeric(args[0])) {
            throw RuntimeError("Runtime Error: net_listen() expects 1 int argument (port)");
        }
        initNetwork();
        socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (IS_INVALID_SOCKET(sock)) return Value::makeInt(-1);
        int opt = 1;
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((uint16_t)toInt64(args[0]));
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            CLOSE_SOCKET(sock);
            return Value::makeInt(-1);
        }
        if (listen(sock, 5) == SOCKET_ERROR) {
            CLOSE_SOCKET(sock);
            return Value::makeInt(-1);
        }
        int64_t id = globalNextSocketId++;
        globalActiveSockets[id] = sock;
        return Value::makeInt(id);
    });

    environment->defineNative("net_accept", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || !isNumeric(args[0])) {
            throw RuntimeError("Runtime Error: net_accept() expects 1 int argument (server_socket_id)");
        }
        int64_t id = toInt64(args[0]);
        auto it = globalActiveSockets.find(id);
        if (it == globalActiveSockets.end()) return Value::makeInt(-1);
        struct sockaddr_in clientAddr{};
#ifdef _WIN32
        int addrLen = sizeof(clientAddr);
#else
        socklen_t addrLen = sizeof(clientAddr);
#endif
        socket_t clientSock = accept(it->second, (struct sockaddr*)&clientAddr, &addrLen);
        if (IS_INVALID_SOCKET(clientSock)) return Value::makeInt(-1);
        int64_t clientId = globalNextSocketId++;
        globalActiveSockets[clientId] = clientSock;
        return Value::makeInt(clientId);
    });

    environment->defineNative("io_read_char", [](const std::vector<Value>&) -> Value {
#ifdef _WIN32
        int ch = _getch();
        return Value::makeStr(std::string(1, static_cast<char>(ch)));
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        if (ch == EOF) return Value::makeStr("");
        return Value::makeStr(std::string(1, static_cast<char>(ch)));
#endif
    });

    environment->defineNative("io_getch", [](const std::vector<Value>&) -> Value {
#ifdef _WIN32
        int ch = _getch();
        return Value::makeStr(std::string(1, static_cast<char>(ch)));
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        if (ch == EOF) return Value::makeStr("");
        return Value::makeStr(std::string(1, static_cast<char>(ch)));
#endif
    });

    environment->defineNative("io_kbhit", [](const std::vector<Value>&) -> Value {
#ifdef _WIN32
        return Value::makeBool(_kbhit() != 0);
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int bytesWaiting = 0;
        ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return Value::makeBool(bytesWaiting > 0);
#endif
    });

    environment->defineNative("console_title", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: console_title() expects 1 str argument (title)");
#ifdef _WIN32
        SetConsoleTitleA(args[0].strVal.c_str());
#else
        std::cout << "\033]0;" << args[0].strVal << "\007";
        std::cout.flush();
#endif
        return Value::makeVoid();
    });

    environment->defineNative("console_cursor", [this](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || !isNumeric(args[0]) || !isNumeric(args[1])) {
            throw RuntimeError("Runtime Error: console_cursor() expects 2 int arguments (row, col)");
        }
        int64_t r = toInt64(args[0]);
        int64_t c = toInt64(args[1]);
        std::cout << "\033[" << r << ";" << c << "H";
        std::cout.flush();
        return Value::makeVoid();
    });

    environment->defineNative("console_color", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: console_color() expects 1 str argument (code)");
        std::cout << args[0].strVal;
        std::cout.flush();
        return Value::makeVoid();
    });

    environment->defineNative("env_get", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: env_get() expects 1 str argument (var_name)");
        const char* val = std::getenv(args[0].strVal.c_str());
        if (val) return Value::makeStr(std::string(val));
        return Value::makeStr("");
    });

    environment->defineNative("env_set", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) {
            throw RuntimeError("Runtime Error: env_set() expects 2 str arguments (var_name, value)");
        }
#ifdef _WIN32
        std::string entry = args[0].strVal + "=" + args[1].strVal;
        return Value::makeBool(_putenv(entry.c_str()) == 0);
#else
        return Value::makeBool(setenv(args[0].strVal.c_str(), args[1].strVal.c_str(), 1) == 0);
#endif
    });

    environment->defineNative("path_join", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2 || args[0].type != DataType::STR || args[1].type != DataType::STR) {
            throw RuntimeError("Runtime Error: path_join() expects 2 str arguments (dir, path)");
        }
        fs::path p1(args[0].strVal);
        fs::path p2(args[1].strVal);
        return Value::makeStr((p1 / p2).string());
    });

    environment->defineNative("path_ext", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: path_ext() expects 1 str argument (path)");
        fs::path p(args[0].strVal);
        return Value::makeStr(p.extension().string());
    });

    environment->defineNative("path_stem", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: path_stem() expects 1 str argument (path)");
        fs::path p(args[0].strVal);
        return Value::makeStr(p.stem().string());
    });

    environment->defineNative("file_is_dir", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_is_dir() expects 1 str argument (path)");
        return Value::makeBool(fs::exists(args[0].strVal) && fs::is_directory(args[0].strVal));
    });

    environment->defineNative("file_is_file", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_is_file() expects 1 str argument (path)");
        return Value::makeBool(fs::exists(args[0].strVal) && fs::is_regular_file(args[0].strVal));
    });

    environment->defineNative("file_lines_count", [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1 || args[0].type != DataType::STR) throw RuntimeError("Runtime Error: file_lines_count() expects 1 str argument (path)");
        std::ifstream f(args[0].strVal);
        if (!f.is_open()) return Value::makeInt(-1);
        int64_t count = 0;
        std::string line;
        while (std::getline(f, line)) {
            count++;
        }
        return Value::makeInt(count);
    });
}

Value Evaluator::evaluate(Expr* expr) {
    return expr->accept(this);
}

void Evaluator::execute(Stmt* stmt) {
    stmt->accept(this);
}

void Evaluator::executeProgram(const std::vector<std::unique_ptr<Stmt>>& program) {
    for (const auto& stmt : program) {
        execute(stmt.get());
    }
}

Value Evaluator::visitLiteralExpr(LiteralExpr* expr) {
    return expr->value;
}

Value Evaluator::visitVariableExpr(VariableExpr* expr) {
    return environment->get(expr->name);
}

Value Evaluator::visitUnaryExpr(UnaryExpr* expr) {
    Value val = evaluate(expr->operand.get());
    if (expr->op == TokenType::TOKEN_NOT) {
        return Value::makeBool(!val.isTruthy());
    }
    if (expr->op == TokenType::TOKEN_MINUS) {
        if (val.type == DataType::FLOAT) {
            return Value::makeFloat(-val.floatVal);
        }
        if (val.type == DataType::INT) {
            return Value::makeInt(-val.intVal);
        }
        if (val.type == DataType::BYTE) {
            return Value::makeInt(-static_cast<int64_t>(val.byteVal));
        }
        throw RuntimeError("Runtime Error: Unary '-' operator requires a numeric operand.");
    }
    if (expr->op == TokenType::TOKEN_HASH) {
        if (val.type == DataType::STR) {
            return Value::makeInt(static_cast<int64_t>(val.strVal.length()));
        }
        if (val.type == DataType::INT) {
            return Value::makeInt(std::abs(val.intVal));
        }
        if (val.type == DataType::FLOAT) {
            return Value::makeFloat(std::fabs(val.floatVal));
        }
        if (val.type == DataType::BYTE) {
            return Value::makeInt(static_cast<int64_t>(val.byteVal));
        }
        throw RuntimeError("Runtime Error: Unary '#' operator requires string or numeric operand.");
    }
    throw RuntimeError("Runtime Error: Unsupported unary operator.");
}

Value Evaluator::visitBinaryExpr(BinaryExpr* expr) {
    if (expr->op == TokenType::TOKEN_AND) {
        Value left = evaluate(expr->left.get());
        if (!left.isTruthy()) return Value::makeBool(false);
        Value right = evaluate(expr->right.get());
        return Value::makeBool(right.isTruthy());
    }
    if (expr->op == TokenType::TOKEN_OR) {
        Value left = evaluate(expr->left.get());
        if (left.isTruthy()) return Value::makeBool(true);
        Value right = evaluate(expr->right.get());
        return Value::makeBool(right.isTruthy());
    }

    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());

    if (expr->op == TokenType::TOKEN_PLUS) {
        if (left.type == DataType::STR || right.type == DataType::STR) {
            return Value::makeStr(left.toString() + right.toString());
        }
        if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
            return Value::makeFloat(toDouble(left) + toDouble(right));
        }
        if (left.type == DataType::BYTE && right.type == DataType::BYTE) {
            return Value::makeByte(static_cast<uint8_t>(left.byteVal + right.byteVal));
        }
        return Value::makeInt(toInt64(left) + toInt64(right));
    }

    if (expr->op == TokenType::TOKEN_MINUS) {
        if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
            return Value::makeFloat(toDouble(left) - toDouble(right));
        }
        if (left.type == DataType::BYTE && right.type == DataType::BYTE) {
            return Value::makeByte(static_cast<uint8_t>(left.byteVal - right.byteVal));
        }
        return Value::makeInt(toInt64(left) - toInt64(right));
    }

    if (expr->op == TokenType::TOKEN_STAR) {
        if (left.type == DataType::STR && (right.type == DataType::INT || right.type == DataType::BYTE)) {
            int64_t count = toInt64(right);
            std::string res = "";
            for (int64_t i = 0; i < count; ++i) res += left.strVal;
            return Value::makeStr(res);
        }
        if ((left.type == DataType::INT || left.type == DataType::BYTE) && right.type == DataType::STR) {
            int64_t count = toInt64(left);
            std::string res = "";
            for (int64_t i = 0; i < count; ++i) res += right.strVal;
            return Value::makeStr(res);
        }
        if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
            return Value::makeFloat(toDouble(left) * toDouble(right));
        }
        if (left.type == DataType::BYTE && right.type == DataType::BYTE) {
            return Value::makeByte(static_cast<uint8_t>(left.byteVal * right.byteVal));
        }
        return Value::makeInt(toInt64(left) * toInt64(right));
    }

    if (expr->op == TokenType::TOKEN_SLASH) {
        if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
            double r = toDouble(right);
            if (r == 0.0) throw RuntimeError("Runtime Error: Division by zero.");
            return Value::makeFloat(toDouble(left) / r);
        }
        int64_t r = toInt64(right);
        if (r == 0) throw RuntimeError("Runtime Error: Division by zero.");
        return Value::makeInt(toInt64(left) / r);
    }

    if (expr->op == TokenType::TOKEN_PERCENT) {
        int64_t r = toInt64(right);
        if (r == 0) throw RuntimeError("Runtime Error: Modulo by zero.");
        return Value::makeInt(toInt64(left) % r);
    }

    if (expr->op == TokenType::TOKEN_EQ) {
        if (left.type == right.type) {
            switch (left.type) {
                case DataType::INT: return Value::makeBool(left.intVal == right.intVal);
                case DataType::FLOAT: return Value::makeBool(left.floatVal == right.floatVal);
                case DataType::BOOL: return Value::makeBool(left.boolVal == right.boolVal);
                case DataType::STR: return Value::makeBool(left.strVal == right.strVal);
                case DataType::BYTE: return Value::makeBool(left.byteVal == right.byteVal);
                case DataType::VOID: return Value::makeBool(true);
            }
        }
        if (isNumeric(left) && isNumeric(right)) {
            return Value::makeBool(toDouble(left) == toDouble(right));
        }
        return Value::makeBool(false);
    }

    if (expr->op == TokenType::TOKEN_NEQ) {
        if (left.type == right.type) {
            switch (left.type) {
                case DataType::INT: return Value::makeBool(left.intVal != right.intVal);
                case DataType::FLOAT: return Value::makeBool(left.floatVal != right.floatVal);
                case DataType::BOOL: return Value::makeBool(left.boolVal != right.boolVal);
                case DataType::STR: return Value::makeBool(left.strVal != right.strVal);
                case DataType::BYTE: return Value::makeBool(left.byteVal != right.byteVal);
                case DataType::VOID: return Value::makeBool(false);
            }
        }
        if (isNumeric(left) && isNumeric(right)) {
            return Value::makeBool(toDouble(left) != toDouble(right));
        }
        return Value::makeBool(true);
    }

    if (expr->op == TokenType::TOKEN_LANGLE) {
        if (left.type == DataType::STR && right.type == DataType::STR) {
            return Value::makeBool(left.strVal < right.strVal);
        }
        if (isNumeric(left) && isNumeric(right)) {
            if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
                return Value::makeBool(toDouble(left) < toDouble(right));
            }
            return Value::makeBool(toInt64(left) < toInt64(right));
        }
        throw RuntimeError("Runtime Error: Operator '<' incompatible with types.");
    }

    if (expr->op == TokenType::TOKEN_LE) {
        if (left.type == DataType::STR && right.type == DataType::STR) {
            return Value::makeBool(left.strVal <= right.strVal);
        }
        if (isNumeric(left) && isNumeric(right)) {
            if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
                return Value::makeBool(toDouble(left) <= toDouble(right));
            }
            return Value::makeBool(toInt64(left) <= toInt64(right));
        }
        throw RuntimeError("Runtime Error: Operator '<=' incompatible with types.");
    }

    if (expr->op == TokenType::TOKEN_RANGLE) {
        if (left.type == DataType::STR && right.type == DataType::STR) {
            return Value::makeBool(left.strVal > right.strVal);
        }
        if (isNumeric(left) && isNumeric(right)) {
            if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
                return Value::makeBool(toDouble(left) > toDouble(right));
            }
            return Value::makeBool(toInt64(left) > toInt64(right));
        }
        throw RuntimeError("Runtime Error: Operator '>' incompatible with types.");
    }

    if (expr->op == TokenType::TOKEN_GE) {
        if (left.type == DataType::STR && right.type == DataType::STR) {
            return Value::makeBool(left.strVal >= right.strVal);
        }
        if (isNumeric(left) && isNumeric(right)) {
            if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
                return Value::makeBool(toDouble(left) >= toDouble(right));
            }
            return Value::makeBool(toInt64(left) >= toInt64(right));
        }
        throw RuntimeError("Runtime Error: Operator '>=' incompatible with types.");
    }

    throw RuntimeError("Runtime Error: Unrecognized binary operator.");
}

Value Evaluator::visitAssignExpr(AssignExpr* expr) {
    Value val = evaluate(expr->value.get());
    environment->assign(expr->name, val);
    return val;
}

Value Evaluator::visitCallExpr(CallExpr* expr) {
    std::vector<Value> argValues;
    for (const auto& arg : expr->args) {
        argValues.push_back(evaluate(arg.get()));
    }

    if (environment->hasNative(expr->callee)) {
        return environment->callNative(expr->callee, argValues);
    }

    FnDeclStmt* fn = environment->getFunction(expr->callee);
    if (!fn) {
        throw RuntimeError("Runtime Error: Call to undefined function '" + expr->callee + "'");
    }

    if (fn->params.size() != argValues.size()) {
        throw RuntimeError("Runtime Error: Function '" + expr->callee + "' expects " + std::to_string(fn->params.size()) + " arguments, got " + std::to_string(argValues.size()));
    }

    auto fnEnv = std::make_shared<Environment>(environment);
    for (size_t i = 0; i < fn->params.size(); ++i) {
        fnEnv->define(fn->params[i].second, argValues[i], fn->params[i].first);
    }

    auto prevEnv = environment;
    environment = fnEnv;
    Value result = Value::makeVoid();
    try {
        execute(fn->body.get());
    } catch (const ReturnSignal& sig) {
        result = sig.value;
    }
    environment = prevEnv;
    return result;
}

Value Evaluator::visitIndexExpr(IndexExpr* expr) {
    Value targetVal = evaluate(expr->target.get());
    Value indexVal = evaluate(expr->index.get());
    if (targetVal.type == DataType::STR) {
        int64_t idx = toInt64(indexVal);
        if (idx < 0 || idx >= static_cast<int64_t>(targetVal.strVal.length())) {
            throw RuntimeError("Runtime Error: String index out of bounds.");
        }
        return Value::makeStr(std::string(1, targetVal.strVal[idx]));
    }
    throw RuntimeError("Runtime Error: Indexing is only supported on str.");
}

Value Evaluator::visitInputExpr(InputExpr* expr) {
    std::string line;
    if (std::getline(std::cin, line)) {
        return Value::makeStr(line);
    }
    return Value::makeStr("");
}

void Evaluator::visitExprStmt(ExprStmt* stmt) {
    evaluate(stmt->expr.get());
}

void Evaluator::visitVarDeclStmt(VarDeclStmt* stmt) {
    Value val;
    if (stmt->init) {
        val = evaluate(stmt->init.get());
    } else {
        switch (stmt->type) {
            case DataType::INT:
                val = Value::makeInt(0);
                break;
            case DataType::FLOAT:
                val = Value::makeFloat(0.0);
                break;
            case DataType::BOOL:
                val = Value::makeBool(false);
                break;
            case DataType::STR:
                val = Value::makeStr("");
                break;
            case DataType::BYTE:
                val = Value::makeByte(0);
                break;
            default:
                val = Value::makeVoid();
                break;
        }
    }
    environment->define(stmt->name, val, stmt->type);
}

void Evaluator::visitOutputStmt(OutputStmt* stmt) {
    Value val = evaluate(stmt->expr.get());
    std::cout << val.toString() << "\n";
    std::cout.flush();
}

void Evaluator::visitRawOutputStmt(RawOutputStmt* stmt) {
    Value val = evaluate(stmt->expr.get());
    std::cout << val.toString();
    std::cout.flush();
}

void Evaluator::visitIfStmt(IfStmt* stmt) {
    Value cond = evaluate(stmt->condition.get());
    if (cond.isTruthy()) {
        execute(stmt->thenBranch.get());
    } else if (stmt->elseBranch) {
        execute(stmt->elseBranch.get());
    }
}

void Evaluator::visitWhileStmt(WhileStmt* stmt) {
    while (evaluate(stmt->condition.get()).isTruthy()) {
        try {
            execute(stmt->body.get());
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue;
        }
    }
}

void Evaluator::visitBlockStmt(BlockStmt* stmt) {
    auto blockEnv = std::make_shared<Environment>(environment);
    auto prevEnv = environment;
    environment = blockEnv;
    try {
        for (const auto& s : stmt->statements) {
            execute(s.get());
        }
    } catch (...) {
        environment = prevEnv;
        throw;
    }
    environment = prevEnv;
}

void Evaluator::visitReturnStmt(ReturnStmt* stmt) {
    Value val = Value::makeVoid();
    if (stmt->expr) {
        val = evaluate(stmt->expr.get());
    }
    throw ReturnSignal(val);
}

void Evaluator::visitBreakStmt(BreakStmt* stmt) {
    throw BreakSignal();
}

void Evaluator::visitContinueStmt(ContinueStmt* stmt) {
    throw ContinueSignal();
}

void Evaluator::visitFnDeclStmt(FnDeclStmt* stmt) {
    environment->defineFunction(stmt->name, stmt);
}

#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <type_traits>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <thread>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <utility>
#include <unordered_map>

struct TypeAndValue {
  std::string type;
  std::string value;
  
  bool operator==(const TypeAndValue& rhs) const {
    return type == rhs.type && value == rhs.value;
  }
};


struct LoggerMessage {
  using TypeAndValueSequence = std::vector<TypeAndValue>;
  using KeyValueStorage = std::unordered_map<std::string, std::string>;

  KeyValueStorage meta;
  TypeAndValueSequence sequence;
};

class LoggerBackend {
public:
  virtual void take(LoggerMessage&& sequence) = 0;
};

class ClogBackend : public LoggerBackend {
public:
  void take(LoggerMessage&& msg) override {
    std::clog << msg.meta["timestamp"] << " <" << msg.meta["thread id"] << "> -- ";
    
    const auto& seq = msg.sequence;
    
    // Format must be ${1:desc}, but we do not check it here
    if (seq[0].type == "fmt") {
      std::string fmt = seq[0].value;
      int inGroup = -1;
      for (size_t i=0; i<fmt.size(); ++i) {
        if (inGroup == -1) {
          if (fmt[i] == '$') {
            assert((i+2) < fmt.size() && fmt[i+1] == '{');
            inGroup = i + 2;
          } else {
            std::clog << fmt[i];
          }
        } else {
          if (fmt[i] == '}') {
            int idx = stoi(fmt.substr(inGroup, i - inGroup));
            std::clog << seq[idx].value;
            inGroup = -1;
          }
          if (fmt[i] == ':') {
            int idx = stoi(fmt.substr(inGroup, i - inGroup));
            std::clog << seq[idx].value;
            inGroup = -1;
            while (fmt[i] != '}') ++i;
            --i;
          }
        }
      }
    } else {
      for (const auto& kv : seq) {
        std::clog << kv.value;
      }
    }
    std::clog << std::endl;
  }
};

class DebugBackend : public LoggerBackend {
public:
  void take(LoggerMessage&& msg) override {
    for (const auto& kv : msg.meta) {
      std::clog << '{' << kv.first << ", " << kv.second << "} ";
    }
    for (const auto& kv : msg.sequence) {
      std::clog << '{' << kv.type << ", " << kv.value << "} ";
    }
    std::clog << std::endl;
  }
};

class FanOutBackend : public LoggerBackend {
public:
  void take(LoggerMessage&& msg) override {
    take(msg);
  }
  
  void take(const LoggerMessage& msg) {
    for (const auto& ch : children) {
      ch->take(LoggerMessage(msg));
    }
  }

  void add(std::shared_ptr<LoggerBackend> lb) {
    children.push_back(lb);
  }
private:
  std::vector<std::shared_ptr<LoggerBackend>> children;
};

constexpr size_t tokenCount(const char* fmt) {
  return (*fmt == '\0') ? size_t(0) : (*fmt == '$') + tokenCount(fmt+1);
}

struct FmtBase {};

template <size_t C>
struct Fmt : public FmtBase {
  template <size_t N>
  Fmt(const char (&s)[N]) : string(s) {
    assert(std::count(string, string + N, '$') == C);
  }
  Fmt(const char* s) : string(s) {
    assert(std::count(string, string + strlen(s), '$') == C);
  }
  const char* string;
};

std::string toString(const char* str) {
  return str;
}

std::string toString(char ch) {
  return std::string(1, ch);
}

template<size_t C>
std::string toString(const Fmt<C>& t) {
  return t.string;
}

std::string toString(const std::chrono::system_clock::time_point& t) {
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  std::tm tm = *std::localtime(&time);
  std::stringstream buf;
  buf << std::put_time(&tm, "%c");
  return buf.str();
}

std::string toString(const std::thread::id& id) {
  std::stringstream buf;
  buf << std::hex << id;
  return buf.str();
}

template<typename T>
constexpr bool isNumberType() {
  return (std::is_integral<T>::value || std::is_floating_point<T>::value)
  && !(std::is_same<char, T>::value
    || std::is_same<unsigned char, T>::value
    || std::is_same<wchar_t, T>::value
    || std::is_same<unsigned wchar_t, T>::value);
}

template <typename T>
typename std::enable_if<isNumberType<T>(), std::string>::type
toString(const T& t) {
  return std::to_string(t);
}

class Logger {
public:
  Logger(std::shared_ptr<LoggerBackend> aBackend) : backend(aBackend) {}
  
  template <
  typename ...Args,
  typename First>
  void log(const First& first, Args... args) const {
    LoggerMessage msg;
    gather(msg, first, args...);
    callBackend(std::move(msg));
  }
  
  template <
  size_t C,
  typename ...Args>
  void log(const Fmt<C>& fmt, Args... args) const {
    static_assert(C == sizeof...(args), "Number of arguments and substitution tokens does not match.");
    LoggerMessage msg;
    msg.sequence.emplace_back(TypeAndValue {"fmt", toString(fmt)});
    gather(msg, args...);
    callBackend(std::move(msg));
  }
private:
  
  inline void callBackend(LoggerMessage&& msg) const {
    LoggerMessage data(msg);
    data.meta["timestamp"] = toString(std::chrono::system_clock::now());
    data.meta["thread id"] = toString(std::this_thread::get_id());
    backend->take(std::move(data));
  }
  
  template <typename ...Args>
  LoggerMessage& gather(LoggerMessage& msg) const {
    return msg;
  }
  
  template <typename Head, typename ...Tail>
  LoggerMessage& gather(LoggerMessage& msg, Head head, Tail... tail) const {
    static_assert(!std::is_base_of<FmtBase, Head>::value,
                  "Format can be only the very first argument");
    msg.sequence.emplace_back(TypeAndValue{"tmp", toString(head)});
    return gather<Tail...>(msg, tail...);
  }
  
  std::shared_ptr<LoggerBackend> backend;
};

#ifdef EXTRACTION

#define DO_PRAGMA(x) _Pragma (#x)
#define MakeFmt(fmt_str) ([](){ DO_PRAGMA(message ("Fmt str " #fmt_str)); return Fmt<tokenCount(fmt_str)>(fmt_str); }())

#else

#define MakeFmt(fmt_str) Fmt<tokenCount(fmt_str)>(fmt_str)

#endif

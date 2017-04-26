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

struct KeyValue {
  std::string key;
  std::string value;
  
  bool operator==(const KeyValue& rhs) const {
    return key == rhs.key && value == rhs.value;
  }
};

using KeyValueSequence = std::vector<KeyValue>;

class LoggerBackend {
public:
  virtual void take(const KeyValueSequence&& sequence) = 0;
  
  static const KeyValue SEPARATOR;
};

const KeyValue LoggerBackend::SEPARATOR = KeyValue{"<separator>", "<meta>"};

class ClogBackend : public LoggerBackend {
public:
  void take(const KeyValueSequence&& seq) override {
    auto sepBack = std::find(seq.rbegin(), seq.rend(), SEPARATOR);
    auto meta = sepBack.base();
    
    for (auto it = meta; it != seq.end(); ++it)
      if (it->key == "thread id") {
        std::clog << '<' << it->value << "> ";
      } else {
        std::clog << it->value << " ";
      }
      
      std::clog << "-- ";
    
    // Format must be ${1:desc}, but we do not check it here
    if (seq[0].key == "fmt") {
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
      for (auto it = seq.begin(); it->key != SEPARATOR.key; ++it) {
        std::clog << it->value;
      }
    }
    std::clog << std::endl;
  }
};

class DebugBackend : public LoggerBackend {
public:
  void take(const KeyValueSequence&& seq) override {
    for (const auto& kv : seq) {
      std::clog << '{' << kv.key << ", " << kv.value << "} ";
    }
    std::clog << std::endl;
  }
};

class FanOutBackend : public LoggerBackend {
public:
  void take(const KeyValueSequence&& seq) override {
    for (const auto& ch : children) {
      ch->take(KeyValueSequence(seq));
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

template<typename T>
std::string toString(const T& s) {
  return s;
}

template<size_t C>
std::string toString(const Fmt<C>& t) {
  return t.string;
}

template<>
std::string toString(const std::chrono::system_clock::time_point& t) {
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  std::tm tm = *std::localtime(&time);
  std::stringstream buf;
  buf << std::put_time(&tm, "%c");
  return buf.str();
}

template<>
std::string toString(const std::thread::id& id) {
  std::stringstream buf;
  buf << std::hex << id;
  return buf.str();
}


template <typename T>
typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, std::string>::type
toString(const T& t) {
  return std::to_string(t);
}

class Logger {
public:
  Logger(std::shared_ptr<LoggerBackend> aBackend) : backend(aBackend) {}
  
  template <
  typename ...Args,
  typename First>
  void debug(const First& first, Args... args) const {
    Kvs kvs;
    gather(kvs, first, args...);
    callBackend(std::move(kvs));
  }
  
  template <
  size_t C,
  typename ...Args>
  void debug(const Fmt<C>& fmt, Args... args) const {
    static_assert(C == sizeof...(args), "Number of arguments and substitution tokens does not match.");
    Kvs kvs;
    kvs.emplace_back(KeyValue {"fmt", toString(fmt)});
    gather(kvs, args...);
    callBackend(std::move(kvs));
  }
private:
  using Kvs = KeyValueSequence;
  
  inline void callBackend(Kvs&& kvs) const {
    Kvs data(kvs);
    data.emplace_back(LoggerBackend::SEPARATOR);
    data.emplace_back(KeyValue{"timestamp", toString(std::chrono::system_clock::now())});
    data.emplace_back(KeyValue{"thread id", toString(std::this_thread::get_id())});
    backend->take(std::move(data));
  }
  
  template <typename ...Args>
  Kvs& gather(Kvs& kvs) const {
    return kvs;
  }
  
  template <typename Head, typename ...Tail>
  Kvs& gather(Kvs& kvs, Head head, Tail... tail) const {
    static_assert(!std::is_base_of<FmtBase, Head>::value,
                  "Format can be only the very first argument");
    kvs.emplace_back(KeyValue{"tmp", toString(head)});
    return gather<Tail...>(kvs, tail...);
  }
  
  std::shared_ptr<LoggerBackend> backend;
};

#ifdef EXTRACTION

#define DO_PRAGMA(x) _Pragma (#x)
#define MakeFmt(fmt_str) ([](){ DO_PRAGMA(message ("Fmt str " #fmt_str)); return Fmt<tokenCount(fmt_str)>(fmt_str); }())

#else

#define MakeFmt(fmt_str) Fmt<tokenCount(fmt_str)>(fmt_str)

#endif

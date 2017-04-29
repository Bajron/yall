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

#include "fmt.hpp"
#include "toString.hpp"

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
  virtual ~LoggerBackend(){};
};

template <typename T>
struct isLogMetaData : std::false_type{};

template <typename T>
typename std::enable_if <isLogMetaData<T>::value, LoggerMessage&>::type
extend(LoggerMessage& msg, T t) {
  msg.meta[typeString(t)] = toString(t);
}

template <typename T>
typename  std::enable_if <!isLogMetaData<T>::value, LoggerMessage&>::type
extend(LoggerMessage& msg, T t) {
  msg.sequence.emplace_back(TypeAndValue{typeString(t), toString(t)});
}

class Logger {
  struct Gatherer {
    Gatherer(Logger& parent) : logger(parent) {}
    ~Gatherer() {
      logger.callBackend(std::move(msg));
    }

    template <typename T>
    Gatherer& operator<<(const T& t) {
      extend(msg, t);
      return *this;
    }

    template <size_t C>
    constexpr Gatherer& operator<<(const ::yall::detail::Fmt<C>&) {
      static_assert(C > 0, "Do not use Fmt in stream interface");
    }

    LoggerMessage msg;
    Logger& logger;
  };
  friend Gatherer;
public:
  explicit Logger(std::shared_ptr<LoggerBackend> aBackend) : backend(aBackend) {}
  Logger() = delete;

  Gatherer operator()() {
    return Gatherer(*this);
  }

  template <typename ...Args, typename First>
  void operator()(const First& first, Args... args) const {
    log(first, args...);
  }

  template <typename ...Args, typename First>
  void log(const First& first, Args... args) const {
    LoggerMessage msg;
    gather(msg, first, args...);
    callBackend(std::move(msg));
  }

  template <size_t C, typename ...Args>
  void operator()(const ::yall::detail::Fmt<C>& fmt, Args... args) const {
    log(fmt, args...);
  }

  template <size_t C, typename ...Args>
  void log(const ::yall::detail::Fmt<C>& fmt, Args... args) const {
    static_assert(C == sizeof...(args), "Number of arguments and substitution tokens does not match.");
    LoggerMessage msg;
    extend(msg, fmt);
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
    static_assert(!std::is_base_of<::yall::detail::FmtBase, Head>::value,
                  "Format can be only the very first argument");
    extend(msg, head);
    return gather<Tail...>(msg, tail...);
  }

  std::shared_ptr<LoggerBackend> backend;
};

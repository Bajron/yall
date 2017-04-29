#pragma once
#include "yall/types.hpp"
#include <iomanip>
#include <iostream>
#include <memory>

namespace yall {

class StreamBackend : public LoggerBackend {
public:
  explicit StreamBackend(std::shared_ptr<std::ostream> ostream) : stream(ostream) {}

  void take(LoggerMessage&& msg) override {
    *stream
      << msg.meta["yall::TimeStamp"]
      << " <" << msg.meta["yall::ThreadId"] << "> "
      << std::setw(8) << msg.meta["yall::Priority"] << " -- ";

    for (auto&& v : msg.sequence) {
      *stream << v.value;
    }
    *stream << std::endl;
  }
private:
  std::shared_ptr<std::ostream> stream;
};

namespace detail {

struct no_delete {
  template <typename T>
  void operator()(T*){}
};

}  // namespace detail

std::shared_ptr<std::ostream> globalStream(std::ostream& s) {
  return std::shared_ptr<std::ostream>(&s, detail::no_delete());
}

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

} // namespace yall


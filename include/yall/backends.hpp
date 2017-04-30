#pragma once
#include "yall/types.hpp"
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

namespace yall {

class MetaFormattingBackend: public LoggerBackend {
public:
  explicit MetaFormattingBackend(std::shared_ptr<LoggerBackend> toDecorate): decorated(toDecorate) {}

  void take(LoggerMessage&& msg) override {
    std::stringstream ss;
    ss << msg.meta["yall::TimeStamp"]
      << " <" << msg.meta["yall::ThreadId"] << "> "
      << std::setw(8) << msg.meta["yall::Priority"] << " -"
      << msg.meta["yall::Prefix"] << "- ";

    msg.sequence.emplace(msg.sequence.begin(), TypeAndValue{"yall::Formatted", ss.str()});
    decorated->take(std::move(msg));
  }
private:
  std::shared_ptr<LoggerBackend> decorated;
};

class StreamBackend : public LoggerBackend {
public:
  explicit StreamBackend(std::shared_ptr<std::ostream> ostream) : stream(ostream) {}

  void take(LoggerMessage&& msg) override {
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

inline std::shared_ptr<std::ostream> globalStream(std::ostream& s) {
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
  FanOutBackend() = default;
  explicit FanOutBackend(const std::initializer_list<std::shared_ptr<LoggerBackend>>& iList)
    : children(iList) {}

  void take(LoggerMessage&& msg) override {
    if (children.empty()) return;
    int n = children.size();
    const LoggerMessage& copy = msg;
    for (int i = 0; i < n - 1; ++i) {
      children[i]->take(LoggerMessage(copy));
    }
    children[n-1]->take(std::move(msg));
  }

  void add(std::shared_ptr<LoggerBackend> lb) {
    children.push_back(lb);
  }
private:
  std::vector<std::shared_ptr<LoggerBackend>> children;
};

class NullBackend : public LoggerBackend {
public:
  void take(LoggerMessage&&) override {}
};

class BackendBuilder {
public:
  BackendBuilder& makeStream(std::shared_ptr<std::ostream> stream) {
    object = std::make_shared<StreamBackend>(stream);
    return *this;
  }
  BackendBuilder& makeConsole(std::ostream& console) {
    return makeStream(globalStream(console));
  }

  template <class T>
  BackendBuilder& decorate() {
    object = std::make_shared<T>(object);
    return *this;
  }
  std::shared_ptr<LoggerBackend> take() {
    return object;
  }
private:
  std::shared_ptr<LoggerBackend> object;
};

} // namespace yall


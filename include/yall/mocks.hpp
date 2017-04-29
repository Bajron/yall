#pragma once

#include <gmock/gmock.h>
#include "yall/backends.hpp"

class MockLoggerBackend: public ::yall::LoggerBackend {
public:
  MOCK_METHOD1(take, void(::yall::LoggerMessage& message));

  void take(::yall::LoggerMessage&& msg) override {
    take(msg);
  }
};

namespace yall {
  void PrintTo(const LoggerMessage& msg, ::std::ostream* os) {
    *os << "LoggerMessage{meta:{";
    for (const auto& kv : msg.meta) {
      *os << "{" << kv.first << ", " << kv.second << "}";
    }
    *os << "}, sequence:{";
    for (const auto& kv : msg.sequence) {
      *os << "{" << kv.type << ", " << kv.value << "}";
    }
    *os << "}}";
  }
}

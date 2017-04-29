#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <chrono>
#include <thread>

namespace yall {
  using TimeStamp = std::chrono::system_clock::time_point;
  using ThreadId = std::thread::id;

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

    bool operator==(const LoggerMessage& rhs) const {
      return meta == rhs.meta && sequence == rhs.sequence;
    }
  };

  class LoggerBackend {
  public:
    virtual void take(LoggerMessage&& sequence) = 0;
    virtual ~LoggerBackend(){};
  };

  template <typename T>
  struct isLogMetaData : std::false_type{};

}

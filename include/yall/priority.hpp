#pragma once

#include "yall/types.hpp"

#include <type_traits>
#include <string>
#include <memory>
#include <stdexcept>

namespace yall {

  enum class Priority {
    Debug,
    Info,
    Warning,
    Error
  };

  template <>
  struct isLogMetaData<Priority> : std::true_type {};

  inline std::string toString(const Priority& p) {
    switch(p) {
      case Priority::Debug: return "debug";
      case Priority::Info: return "info";
      case Priority::Warning: return "warning";
      case Priority::Error: return "error";
    }
    throw std::logic_error("enum not handled, where is your Werror?");
  }

  inline std::string typeString(const Priority&) {
    return "yall::Priority";
  }

  class PriorityDecoratingBackend: public LoggerBackend {
  public:
    PriorityDecoratingBackend(
      std::shared_ptr<LoggerBackend> toDecorate,
      Priority priorityToAdd
    ) : decorated(toDecorate), priority(priorityToAdd) {

    }

    void take(LoggerMessage&& msg) override {
      msg.meta["yall::Priority"] = toString(priority);
      decorated->take(std::move(msg));
    }

  private:
    std::shared_ptr<LoggerBackend> decorated;
    Priority priority;
  };
}

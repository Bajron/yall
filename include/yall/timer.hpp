#pragma once

#include "yall/logger.hpp"
#include <chrono>

namespace yall {

template <class T = std::chrono::steady_clock>
struct Timer {
  Logger logger;
  typename T::time_point last;

  Timer(Logger logger): logger(logger), last(T::now()) {
    logger("Timer starts");
  }

  void operator()(const std::string& message = "") {
    auto now = T::now();
    logger() << message << " "
      << std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() << " ms";
    last = now;
  }
};

}

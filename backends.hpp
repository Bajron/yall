#pragma once
#include "logger.hpp"
#include <iomanip>
#include <iosfwd>
#include <memory>

class FmtEvaluatingBackend: public LoggerBackend {
public:
  explicit FmtEvaluatingBackend(std::shared_ptr<LoggerBackend> toDecorate):
    decorated(toDecorate) {}

  void take(LoggerMessage&& msg) override {
    auto& seq = msg.sequence;
    if (seq[0].type == "Fmt") {
      const std::string& fmt = seq[0].value;

      std::string str;

      const char* ch = &fmt[0];
      const char* end = ch + fmt.size();
      while (ch != end) {
        if (*ch == '$') {
          ++ch;
          auto indexAndEnd = ::yall::detail::readPlaceholder(ch);
          str += seq[indexAndEnd.first].value;
          ch = indexAndEnd.second;
        } else {
          str += *ch++;
        }
      }
      seq.clear();
      seq.emplace_back(TypeAndValue{"Formatted", std::move(str)});
    }
    decorated->take(std::move(msg));
  }
private:
    std::shared_ptr<LoggerBackend> decorated;
};

class StreamBackend : public LoggerBackend {
public:
  explicit StreamBackend(std::shared_ptr<std::ostream> ostream) : stream(ostream) {}

  void take(LoggerMessage&& msg) override {
    *stream
      << msg.meta["timestamp"]
      << " <" << msg.meta["thread id"] << "> "
      << std::setw(8) << msg.meta["priority"] << " -- ";

    for (auto&& v : msg.sequence) {
      *stream << v.value;
    }
    *stream << std::endl;
  }
private:
  std::shared_ptr<std::ostream> stream;
};

struct no_delete {
  template <typename T>
  void operator()(T*){}
};

std::shared_ptr<std::ostream> globalStream(std::ostream& s) {
  return std::shared_ptr<std::ostream>(&s, no_delete());
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

enum class Priority {
  Debug,
  Info,
  Warning,
  Error
};

template <>
struct isLogMetaData<Priority> : std::true_type {};

std::string toString(const Priority& p) {
  switch(p) {
    case Priority::Debug: return "debug";
    case Priority::Info: return "info";
    case Priority::Warning: return "warning";
    case Priority::Error: return "error";
  }
  throw std::logic_error("enum not handled, where is your Werror?");
}

std::string typeString(const Priority&) {
  return "priority";
}

class PriorityDecoratingBackend: public LoggerBackend {
public:
  PriorityDecoratingBackend(
    std::shared_ptr<LoggerBackend> toDecorate,
    Priority priorityToAdd
  ) : decorated(toDecorate), priority(priorityToAdd) {

  }

  void take(LoggerMessage&& msg) override {
    msg.meta["priority"] = toString(priority);
    decorated->take(std::move(msg));
  }

private:
  std::shared_ptr<LoggerBackend> decorated;
  Priority priority;
};




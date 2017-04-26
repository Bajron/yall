#pragma once
#include "logger.hpp"
#include <iomanip>

class ClogBackend : public LoggerBackend {
public:
  void take(LoggerMessage&& msg) override {
    std::clog << msg.meta["timestamp"] << " <" << msg.meta["thread id"] << "> "
      << std::setw(8) << msg.meta["priority"] << " -- ";
    
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

enum class Priority {
  Debug,
  Info,
  Warning,
  Error
};

std::string toString(const Priority& p) {
  switch(p) {
    case Priority::Debug: return "debug";
    case Priority::Info: return "info";
    case Priority::Warning: return "warning";
    case Priority::Error: return "error";
  }
  throw std::logic_error("enum not handled, where is your Werror?");
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


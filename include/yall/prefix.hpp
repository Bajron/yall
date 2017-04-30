#pragma once
#include "yall/logger.hpp"

namespace yall {

class PrefixDecoratingBackend: public LoggerBackend {
public:
  PrefixDecoratingBackend(std::shared_ptr<LoggerBackend> toDecorate, const std::string& prefix):
    decorated(toDecorate), prefix(prefix) {}

  void take(LoggerMessage&& msg) override {
    msg.meta["yall::Prefix"] = prefix;
    decorated->take(std::move(msg));
  }

  std::shared_ptr<PrefixDecoratingBackend> getChild(const std::string& name) const {
    return std::make_shared<PrefixDecoratingBackend>(decorated, prefix + '.' + name);
  }
private:
  std::shared_ptr<LoggerBackend> decorated;
  std::string prefix;
};

class PrefixedLogger: public Logger {
  PrefixedLogger(std::shared_ptr<PrefixDecoratingBackend> backend):
    Logger(backend), prefixBackend(backend) {}

public:
  PrefixedLogger(std::shared_ptr<LoggerBackend> backend):
    PrefixedLogger(std::make_shared<PrefixDecoratingBackend>(backend, "root")) {}

  PrefixedLogger child(const std::string& name) {
    return PrefixedLogger(prefixBackend->getChild(name));
  }
private:
  std::shared_ptr<PrefixDecoratingBackend> prefixBackend;
};

} // namespace yall

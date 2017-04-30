#include <benchmark/benchmark.h>
#include "yall/logger.hpp"
#include "yall/backends.hpp"
#include <sstream>
#include <cstdio>
#include <vector>

using namespace yall;

namespace {

static void BM_LoggerStream(benchmark::State& state) {
  auto stream = std::make_shared<std::stringstream>();
  Logger log(BackendBuilder().makeStream(stream).take());
  while (state.KeepRunning())
    log.log("test");
}
BENCHMARK(BM_LoggerStream);

// Not so accurate (timestamps and stuff)

static void BM_LogStream(benchmark::State& state) {
  std::stringstream stream;
  while (state.KeepRunning())
    stream << "test" << std::endl;
}
BENCHMARK(BM_LogStream);

static void BM_LogSprintf(benchmark::State& state) {
  std::vector<char> buf(1024);
  size_t off = 0;
  while (state.KeepRunning()) {
    auto c = snprintf(&buf[off], buf.size() - off, "test\n");
    off += c;
    if (buf.size() - off < 64)
      buf.resize(buf.size() + 1024);
  }
}
BENCHMARK(BM_LogSprintf);

}

BENCHMARK_MAIN();

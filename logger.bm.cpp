#include <benchmark/benchmark.h>
#include "yall/logger.hpp"
#include "yall/backends.hpp"
#include <sstream>
#include <cstdio>
#include <vector>
#include <thread>

#include <sys/types.h>
#include <unistd.h>

using namespace yall;

namespace {

static void BM_LoggerStream(benchmark::State& state) {
  auto stream = std::make_shared<std::stringstream>();
  Logger log(BackendBuilder().makeStream(stream).take());
  while (state.KeepRunning())
    log.log("test");
}
BENCHMARK(BM_LoggerStream);

static void BM_LogStream(benchmark::State& state) {
  std::stringstream stream;
  while (state.KeepRunning())
    stream << toString(std::chrono::system_clock::now())
      << " <" << std::this_thread::get_id()  << "> "
      << "test" << std::endl;
}
BENCHMARK(BM_LogStream);

static void BM_LogSprintf(benchmark::State& state) {
  std::vector<char> buf(1024);
  size_t off = 0;
  while (state.KeepRunning()) {
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    auto c = snprintf(&buf[off], buf.size() - off,
      "%s <%d> test\n",
      asctime (timeinfo), (int)getpid()
    );
    off += c;
    if (buf.size() - off < 256)
      buf.resize(buf.size() + 2048);
  }
}
BENCHMARK(BM_LogSprintf);

}

BENCHMARK_MAIN();

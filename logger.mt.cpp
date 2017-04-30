#include "yall/logger"
#include "yall/backends.hpp"
#include "yall/prefix.hpp"
#include "yall/timer.hpp"

enum TestLogs {
  FIRST = 0,
  Zero, One, Two, NoImpl,
  LAST
};

constexpr const char* tr(TestLogs l) {
  return (l == Zero) ? " Zero "
    :    (l == One) ? "One ${1} log"
    :    (l == Two) ? "Two ${2} ${1} log"
    : throw std::logic_error("Translation not found");
}

using namespace yall;

int main() {
  auto be = std::make_shared<FanOutBackend>();
  be->add(std::make_shared<DebugBackend>());
  be->add(BackendBuilder()
    .makeConsole(std::clog)
    .decorate<MetaFormattingBackend>()
    .decorate<FmtEvaluatingBackend>()
    .take()
  );
  Logger log(be);

  Timer<> tm(log);

  log.log("x", 'c', "x", 1, 1.0);

  log.log(MakeFmt("${1}"), "test");
  log.log(MakeFmt(tr(One)), "test");
  log.log(MakeFmt(tr(Zero)));
  log.log(MakeFmt(tr(Two)), "one", "two");

  std::string test = "foo";
  log() << "Wow!" << ' ' << test;

  static_assert(isLogMetaData<Priority>::value, "you failed at traits");
  log() << "Result is " << 10 << Priority::Warning;

  tm("Playing with plain logger done");

  Logger debug(std::make_shared<PriorityDecoratingBackend>(be, Priority::Debug));

  debug(MakeFmt("Debugging: ${1}!"), "Heeeyy");
  debug() << "So " << "much" << ' ' << "freedom";
  debug("So ", "much", ' ', "freedom");

  debug() << Priority::Error << "Weird?";
  log() << Priority::Error << "Weird..." << Priority::Warning;

  tm("Debug logger done");

  PrefixedLogger root(be);
  auto child = root.child("child");

  root() << "Hello";
  child() << "ohai!";

  tm("Prefixing games done");

  //  log.log(MakeFmt(tr(One)));
  //  log.log(MakeFmt(tr(Zero)), " x ");
  //  log.log(MakeFmt(tr(NoImpl)));

  return 0;
}

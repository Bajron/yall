// flags: -D EXTRACTION
#include "logger.hpp"
#include "backends.hpp"

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

int main() {
  auto be = std::make_shared<FanOutBackend>();
  be->add(std::make_shared<DebugBackend>());
  be->add(
    std::make_shared<FmtEvaluatingBackend>(
      std::make_shared<StreamBackend>(globalStream(std::clog))
    )
  );
  Logger log(be);

  log.log("x", 'c', "x", 1, 1.0);

  log.log(MakeFmt("${1}"), "test");
  log.log(MakeFmt(tr(One)), "test");
  log.log(MakeFmt(tr(Zero)));
  log.log(MakeFmt(tr(Two)), "one", "two");

  log() << "Wow!";

  static_assert(isLogMetaData<Priority>::value, "you failed at traits");
  log() << "Result is " << 10 << Priority::Warning;

  Logger debug(std::make_shared<PriorityDecoratingBackend>(be, Priority::Debug));

  debug(MakeFmt("Debugging: ${1}!"), "Heeeyy");
  debug() << "So " << "much" << ' ' << "freedom";
  debug("So ", "much", ' ', "freedom");


  //  log.log(MakeFmt(tr(One)));
  //  log.log(MakeFmt(tr(Zero)), " x ");
  //  log.log(MakeFmt(tr(NoImpl)));

  return 0;
}

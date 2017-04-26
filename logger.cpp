// flags: -D EXTRACTION
#include "logger.hpp"

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
  be->add(std::make_shared<ClogBackend>());
  be->add(std::make_shared<DebugBackend>());
  Logger log(be);
  
  log.debug("x", "x");

  log.debug(MakeFmt("${1}"), "test");
  log.debug(MakeFmt(tr(One)), "test");
  log.debug(MakeFmt(tr(Zero)));
  log.debug(MakeFmt(tr(Two)), "one", "two");
  
//  log.debug(MakeFmt(tr(One)));
//  log.debug(MakeFmt(tr(Zero)), " x ");
//  log.debug(MakeFmt(tr(NoImpl)));


  static_assert(tokenCount("$ $") == 2);

  return 0;
}

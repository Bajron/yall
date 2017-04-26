#pragma once

struct FmtBase {};

template <size_t C>
struct Fmt : public FmtBase {
  template <size_t N>
  Fmt(const char (&s)[N]) : string(s) {
    assert(std::count(string, string + N, '$') == C);
  }
  Fmt(const char* s) : string(s) {
    assert(std::count(string, string + strlen(s), '$') == C);
  }
  const char* string;
};

constexpr size_t tokenCount(const char* fmt) {
  return (*fmt == '\0') ? size_t(0) : (*fmt == '$') + tokenCount(fmt+1);
}

#ifdef EXTRACTION

#define DO_PRAGMA(x) _Pragma (#x)
#define MakeFmt(fmt_str) ([](){ DO_PRAGMA(message ("Fmt str " #fmt_str)); return Fmt<tokenCount(fmt_str)>(fmt_str); }())

#else

#define MakeFmt(fmt_str) Fmt<tokenCount(fmt_str)>(fmt_str)

#endif

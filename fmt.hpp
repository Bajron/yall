#pragma once
#include <stdexcept>
#include <string>
#include <algorithm>

namespace yall {
namespace detail {

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

constexpr size_t readNumber(const char* start, const char* end) {
  size_t ret = 0;

  auto it = start;
  while (it != end) {
    if (!('0' <= *it && *it <= '9'))
      throw std::logic_error("Expected a number");

    ret *= 10;
    ret += *it - '0';

    ++it;
  }
  return ret;
}

constexpr std::pair<size_t, const char*> readPlaceholder(const char* it) {
  if (*it != '{')
    throw std::logic_error("Expected '{' after '$'");
    ++it;

    auto start = it;
    while (*it != ':' && *it != '}' && *it != '\0') ++it;

    if (*it == '\0')
      throw std::logic_error("Closing brace not found");

    auto end = it;
    auto ret = readNumber(start, end);

    while (*it != '}' && *it != '\0') ++it;

    if (*it == '\0')
      throw std::logic_error("Closing brace not found");

    ++it;

    return std::make_pair(ret, it);
}

#if __cplusplus < 199711L
// ancient
#elif __cplusplus <  201103L
// 98
#elif __cplusplus < 201402L
// 11

constexpr size_t placeholderCount(const char* fmt) {
  return (*fmt == '\0')
    ? size_t(0)
    : (*fmt == '$') + placeholderCount(fmt + 1);
}

#else

constexpr size_t placeholderCount(const char* fmt) {
  size_t maxFound = 0;

  const char* it = fmt;
  while (*it != '\0') {
    if (*it == '$') {
      ++it;
      auto valueAndIt = readPlaceholder(it);
      size_t found = valueAndIt.first;
      if (found > maxFound)
        maxFound = found;
      it = valueAndIt.second;
    } else {
      ++it;
    }
  }
  return maxFound;
}

#endif

} // detail
} // yall

#ifdef YALL_EXTRACTION

#define DO_PRAGMA(x) _Pragma (#x)
#define MakeFmt(fmt_str) ([](){ DO_PRAGMA(message ("Fmt str " #fmt_str)); return Fmt<placeholderCount(fmt_str)>(fmt_str); }())

#else

#define MakeFmt(fmt_str) ::yall::detail::Fmt<::yall::detail::placeholderCount(fmt_str)>(fmt_str)

#endif

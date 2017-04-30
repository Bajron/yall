#pragma once

#include <sstream>
#include <iomanip>

namespace yall {

template <typename T>
std::string typeString(const T& t) {
  return typeid(t).name();
}

inline std::string toString(const char* str) {
  return str;
}

inline std::string toString(const std::string& str) {
  return str;
}


inline std::string toString(char ch) {
  return std::string(1, ch);
}

template<typename T>
constexpr bool isNumberType() {
  return (std::is_integral<T>::value || std::is_floating_point<T>::value)
  && !(std::is_same<char, T>::value
  || std::is_same<unsigned char, T>::value
  || std::is_same<wchar_t, T>::value
  || std::is_same<unsigned wchar_t, T>::value);
}

template <typename T>
typename std::enable_if<isNumberType<T>(), std::string>::type
toString(const T& t) {
  return std::to_string(t);
}

inline std::string toString(const std::chrono::system_clock::time_point& t) {
  auto milis = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count();
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  std::tm tm = *std::localtime(&time);
  std::stringstream buf;
  buf << std::put_time(&tm, "%F %T")
    << '.' << std::setfill('0') << std::setw(3) << milis % 1000;
  return buf.str();
}

inline std::string toString(const std::thread::id& id) {
  std::stringstream buf;
  buf << std::hex << id;
  return buf.str();
}

} // namespace yall

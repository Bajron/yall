#pragma once

std::string toString(const char* str) {
  return str;
}

std::string toString(char ch) {
  return std::string(1, ch);
}

template<size_t C>
std::string toString(const Fmt<C>& t) {
  return t.string;
}

std::string toString(const std::chrono::system_clock::time_point& t) {
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  std::tm tm = *std::localtime(&time);
  std::stringstream buf;
  buf << std::put_time(&tm, "%c");
  return buf.str();
}

std::string toString(const std::thread::id& id) {
  std::stringstream buf;
  buf << std::hex << id;
  return buf.str();
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

/**
 * Functions and macros for throwing exceptions while supporting platforms that do not have
 * exceptions.
 *
 * The except() function and its overloads log a message and throw an exception. On platforms
 * without exception support, this exception is treated as an uncaught exception and results in
 * terminating the program.
 */
#pragma once

#include <stdexcept>
#include <system_error>

#ifdef __has_include
#if __has_include(<string>)
#include <string>
// Create our own feature testing symbol for string support. Note that this symbol follows the
// pattern of other feature testing macros but is not part of the standard.
#define __cpp_lib_string 1
#endif
#endif

constexpr bool has_exception_support() {
#if defined(__cpp_exceptions) && !defined(NO_EXCEPTIONS)
  return true;
#else
  return false;
#endif
}

namespace tvsc {

#ifdef __cpp_lib_source_location

template <typename ExceptionT>
[[noreturn]] inline void except(
    const char* const msg, const std::source_location& location = std::source_location::current()) {
  if constexpr (has_exception_support()) {
    Serial.print("[");
    Serial.print(location.function_name());
    Serial.print(" (");
    Serial.print(location.file_name());
    Serial.print(":");
    Serial.print(location.line());
    Serial.print(")] -- ");
    Serial.println(msg);
    throw ExceptionT(msg);
  } else {
    Serial.print("[");
    Serial.print(location.function_name());
    Serial.print(" (");
    Serial.print(location.file_name());
    Serial.print(":");
    Serial.print(location.line());
    Serial.print(")] -- ");
    Serial.println(msg);
    abort();
  }
}

[[noreturn]] inline void except(
    const char* const msg, const std::error_code& ec,
    const std::source_location& location = std::source_location::current()) {
  if constexpr (has_exception_support()) {
    Serial.print("[");
    Serial.print(location.function_name());
    Serial.print(" (");
    Serial.print(location.file_name());
    Serial.print(":");
    Serial.print(location.line());
    Serial.print(")] -- ");
    Serial.print(msg);
    Serial.print(" (");
    Serial.print(ec.message().c_str());
    Serial.println(")");
    throw ExceptionT(ec, msg);
  } else {
    Serial.print("[");
    Serial.print(location.function_name());
    Serial.print(" (");
    Serial.print(location.file_name());
    Serial.print(":");
    Serial.print(location.line());
    Serial.print(")] -- ");
    Serial.print(msg);
    Serial.print(" (");
    Serial.print(ec.message().c_str());
    Serial.println(")");
    abort();
  }
}

#ifdef __cpp_lib_string

template <typename ExceptionT>
[[noreturn]] inline void except(const std::string& msg, const std::source_location& location =
                                                            std::source_location::current()) {
  except<ExceptionT>(msg.c_str(), location);
}

template <typename ExceptionT>
[[noreturn]] inline void except(
    const std::string& msg, const std::error_code& ec,
    const std::source_location& location = std::source_location::current()) {
  except<ExceptionT>(msg.c_str(), ec, location);
}

#endif  // __cpp_lib_string

#else  // __cpp_lib_source_location

template <typename ExceptionT>
[[noreturn]] inline void except(const char* const msg) {
  if constexpr (has_exception_support()) {
    Serial.println(msg);
    throw ExceptionT(msg);
  } else {
    Serial.println(msg);
    abort();
  }
}

template <typename ExceptionT>
[[noreturn]] inline void except(const char* const msg, const std::error_code& ec) {
  if constexpr (has_exception_support()) {
    Serial.print(msg);
    Serial.print(" (");
    Serial.print(ec.message().c_str());
    Serial.println(")");
    throw ExceptionT(ec, msg);
  } else {
    Serial.print(msg);
    Serial.print(" (");
    Serial.print(ec.message().c_str());
    Serial.println(")");
    abort();
  }
}

#ifdef __cpp_lib_string

template <typename ExceptionT>
[[noreturn]] inline void except(const std::string& msg) {
  except<ExceptionT>(msg.c_str());
}

template <typename ExceptionT>
[[noreturn]] inline void except(const std::string& msg, const std::error_code& ec) {
  except<ExceptionT>(msg.c_str(), ec);
}

#endif  // __cpp_lib_string

#endif  // __cpp_lib_source_location

}  // namespace tvsc

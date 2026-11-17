/**
 * Functions and macros for throwing exceptions while supporting platforms that do not have
 * exceptions.
 *
 * The except() function and its overloads log a message and throw an exception. On platforms
 * without exception support, this exception is treated as an uncaught exception and results in
 * terminating the program.
 */
#pragma once

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#ifdef __has_include
#if __has_include(<string>)
#include <string>
// Create our own feature testing symbol for string support. Note that this symbol follows the
// pattern of other feature testing macros but is not part of the standard.
#define __cpp_lib_string 1
#endif
#endif

#include "error/fault_message.h"

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
  // Clear any previous message, possibly initializing the underlying buffer.
  error::fault_message.clear();
  error::fault_message.append("[");
  error::fault_message.append(location.function_name());
  error::fault_message.append(" (");
  error::fault_message.append(location.file_name());
  error::fault_message.append(":");
  error::fault_message.append(location.line());
  error::fault_message.append(")] -- ");
  error::fault_message.append(msg);
  if constexpr (has_exception_support()) {
    throw ExceptionT(msg);
  } else {
    abort();
  }
}

[[noreturn]] inline void except(
    const char* const msg, const std::error_code& ec,
    const std::source_location& location = std::source_location::current()) {
  error::fault_message.clear();
  error::fault_message.append("[");
  error::fault_message.append(location.function_name());
  error::fault_message.append(" (");
  error::fault_message.append(location.file_name());
  error::fault_message.append(":");
  error::fault_message.append(location.line());
  error::fault_message.append(")] -- ");
  error::fault_message.append(msg);
  error::fault_message.append(" (");
  error::fault_message.append(ec.message().c_str());
  error::fault_message.append(")");
  if constexpr (has_exception_support()) {
    throw ExceptionT(ec, msg);
  } else {
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
  error::fault_message.clear();
  error::fault_message.append(msg);
  if constexpr (has_exception_support()) {
    throw ExceptionT(msg);
  } else {
    abort();
  }
}

template <typename ExceptionT>
[[noreturn]] inline void except(const char* const msg, const std::error_code& ec) {
  error::fault_message.clear();
  error::fault_message.append(msg);
  error::fault_message.append(" (");
  error::fault_message.append(ec.message().c_str());
  error::fault_message.append(")");
  if constexpr (has_exception_support()) {
    throw ExceptionT(ec, msg);
  } else {
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

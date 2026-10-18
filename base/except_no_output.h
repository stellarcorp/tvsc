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

#include "platforms/linker_sections.h"

constexpr bool has_exception_support() {
#if defined(__cpp_exceptions) && !defined(NO_EXCEPTIONS)
  return true;
#else
  return false;
#endif
}

namespace tvsc {

template <std::size_t TOTAL_SIZE, typename CharT = char, typename Traits = std::char_traits<CharT>>
class FixedBuffer final {
 private:
  static constexpr std::size_t CAPACITY{TOTAL_SIZE - sizeof(std::size_t)};
  std::size_t size_{};
  std::array<CharT, CAPACITY> buffer_{};

 public:
  static constexpr std::size_t capacity() noexcept { return CAPACITY; }

  void clear() noexcept {
    size_ = 0;
    buffer_[0] = CharT{};
  }

  FixedBuffer& append(CharT c) noexcept {
    if (size_ + 1 < CAPACITY) {
      buffer_[size_++] = c;
      buffer_[size_] = CharT{};
    }
    return *this;
  }

  FixedBuffer& append(std::basic_string_view<CharT, Traits> sv) noexcept {
    if (size_ + 1 < CAPACITY) {
      std::size_t available{CAPACITY - 1 - size_};
      std::size_t to_copy = std::min(sv.size(), available);
      Traits::copy(buffer_.data() + size_, sv.data(), to_copy);
      size_ += to_copy;
      buffer_[size_] = CharT{};
    }
    return *this;
  }

  template <std::integral T>
  FixedBuffer& append(T val) noexcept {
    if (size_ + 1 < CAPACITY) {
      // Target the available capacity, leaving at least one byte for null-termination.
      CharT* first{buffer_.data() + size_};
      CharT* last{buffer_.data() + CAPACITY - 1};

      auto [ptr, ec] = std::to_chars(first, last, val);
      if (ec == std::errc{}) {
        size_ += static_cast<std::size_t>(ptr - first);
        buffer_[size_] = CharT{};
      }
    }
    return *this;
  }

  operator std::basic_string_view<CharT, Traits>() const noexcept {
    return std::basic_string_view<CharT, Traits>(buffer_.data(), size_);
  }

  const CharT* data() const noexcept { return buffer_.data(); }
  std::size_t size() const noexcept { return size_; }
};

extern FixedBuffer<256> fault_message;

#ifdef __cpp_lib_source_location

template <typename ExceptionT>
[[noreturn]] inline void except(
    const char* const msg, const std::source_location& location = std::source_location::current()) {
  // Clear any previous message, possibly initializing the underlying buffer.
  fault_message.clear();
  fault_message.append("[");
  fault_message.append(location.function_name());
  fault_message.append(" (");
  fault_message.append(location.file_name());
  fault_message.append(":");
  fault_message.append(location.line());
  fault_message.append(")] -- ");
  fault_message.append(msg);
  if constexpr (has_exception_support()) {
    throw ExceptionT(msg);
  } else {
    abort();
  }
}

[[noreturn]] inline void except(
    const char* const msg, const std::error_code& ec,
    const std::source_location& location = std::source_location::current()) {
  fault_message.clear();
  fault_message.append("[");
  fault_message.append(location.function_name());
  fault_message.append(" (");
  fault_message.append(location.file_name());
  fault_message.append(":");
  fault_message.append(location.line());
  fault_message.append(")] -- ");
  fault_message.append(msg);
  fault_message.append(" (");
  fault_message.append(ec.message().c_str());
  fault_message.append(")");
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
  fault_message.clear();
  fault_message.append(msg);
  if constexpr (has_exception_support()) {
    throw ExceptionT(msg);
  } else {
    abort();
  }
}

template <typename ExceptionT>
[[noreturn]] inline void except(const char* const msg, const std::error_code& ec) {
  fault_message.clear();
  fault_message.append(msg);
  fault_message.append(" (");
  fault_message.append(ec.message().c_str());
  fault_message.append(")");
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

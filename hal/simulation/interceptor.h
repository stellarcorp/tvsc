#pragma once

#include <functional>
#include <iostream>

#include "hal/chrono_utils.h"

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace tvsc::hal::simulation {

template <typename Interface, typename ClockType>
class Interceptor : public Interface {
 private:
  Interface* instance_;

 protected:
  using InterfaceType = Interface;

  template <typename Fn, typename... Args>
  auto call(Fn&& fn, Args&&... args) {
    return std::invoke(std::forward<Fn>(fn), instance_, std::forward<Args>(args)...);
  }

#if __cpp_lib_source_location >= 201907L

  void log_fn(const std::source_location& location = std::source_location::current()) {
    // TODO(james): Replace with write to protocol buffer for better analysis.
    std::cout << to_string(ClockType::now()) << " -- " << filename << ":" << line_number << " -- "
              << function_name << "()\n";
  }

#else

  void log_fn(const char* filename, uint32_t line_number, const char* function_name) {
    // TODO(james): Replace with write to protocol buffer for better analysis.
    std::cout << to_string(ClockType::now()) << " -- " << filename << ":" << line_number << " -- "
              << function_name << "()\n";
  }

#endif

 public:
  explicit Interceptor(Interface& instance) : instance_(&instance) {}
  virtual ~Interceptor() = default;
};

// Provide a macro so that we can use the same code with and without support for
// std::source_location. Note that this macro only works within the context of Interceptor and its
// subclasses.
#if __cpp_lib_source_location >= 201907L
#define LOG_FN() this->log_fn()
#else
#define LOG_FN() this->log_fn((__FILE__), (__LINE__), (__PRETTY_FUNCTION__))
#endif

}  // namespace tvsc::hal::simulation

#pragma once

#include <functional>
#include <iostream>

#include "hal/chrono_utils.h"
#include "hal/simulation/logger.h"

namespace tvsc::hal::simulation {

template <typename InterfaceType, typename ClockType>
class Interceptor : public InterfaceType {
 private:
  InterfaceType* instance_;
  Logger<ClockType>* logger_;

 protected:
  template <typename Fn, typename... Args>
  auto call(Fn&& fn, Args&&... args) {
    return std::invoke(std::forward<Fn>(fn), instance_, std::forward<Args>(args)...);
  }

#if __cpp_lib_source_location >= 201907L

  void log_fn(const std::source_location& location = std::source_location::current()) {
    logger_->log_fn(location);
  }

#else

  void log_fn(const char* filename, uint32_t line_number, const char* function_name) {
    logger_->log_fn(filename, line_number, function_name);
  }

#endif

 public:
  explicit Interceptor(InterfaceType& instance, Logger<ClockType>& logger)
      : instance_(&instance), logger_(&logger) {}
  virtual ~Interceptor() = default;
};

// Provide a macro so that we can use the same code with and without support for
// std::source_location. Note that this macro only works within the context of Interceptor and its
// subclasses.
#if __cpp_lib_source_location >= 201907L
#define LOG_FN() this->log_fn()
#else
#define LOG_FN() this->log_fn((__FILE__), (__LINE__), (__func__))
#endif

}  // namespace tvsc::hal::simulation

#pragma once

#include <functional>
#include <iostream>

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace tvsc::hal {

template <typename Interface>
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
    std::cout << filename << ":" << line_number << " -- " << function_name << "()\n";
  }

#else

  void log_fn(const char* function_name, const char* filename = __FILE__,
              uint32_t line_number = __LINE__) {
    // TODO(james): Replace with write to protocol buffer for better analysis.
    std::cout << filename << ":" << line_number << " -- " << function_name << "()\n";
  }

#endif

 public:
  explicit Interceptor(Interface& instance) : instance_(&instance) {}
  virtual ~Interceptor() = default;
};

}  // namespace tvsc::hal

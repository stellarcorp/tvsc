#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

namespace tvsc::control {

template <typename T>
class Parameter {
 public:
  virtual ~Parameter() = default;
  virtual bool is_allowed(const T& value) const = 0;
};

namespace impl {

template <typename T>
class CombinedParameters final : public Parameter<T> {
 private:
  const std::vector<std::unique_ptr<Parameter<T>>> parameters_;

 public:
  CombinedParameters(std::vector<std::unique_ptr<Parameter<T>>>&& parameters)
      : parameters_(std::move(parameters)) {}
  CombinedParameters(CombinedParameters&& rhs) : parameters_(std::move(rhs.parameters_)) {}

  CombinedParameters& operator=(CombinedParameters&& rhs) {
    parameters_ = std::move(rhs.parameters_);
    return *this;
  }

  bool is_allowed(const T& value) const override {
    for (const auto& p : parameters_) {
      if (p->is_allowed(value)) {
        return true;
      }
    }
    return false;
  }
};

}  // namespace impl

template <typename T, typename... Args>
std::unique_ptr<Parameter<T>> combine(Args... parameters) {
  std::vector<std::unique_ptr<Parameter<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(parameters)), ...);
  return std::unique_ptr<Parameter<T>>(new impl::CombinedParameters<T>(std::move(v)));
}

template <typename T>
std::unique_ptr<Parameter<T>> exclude(std::unique_ptr<Parameter<T>>&& parameter,
                                      std::unique_ptr<Parameter<T>>&& excluded);

}  // namespace tvsc::control

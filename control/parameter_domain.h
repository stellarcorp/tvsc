#pragma once

#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

namespace tvsc::control {

  namespace impl {
    
template <typename T>
T default_precision() {
  return 1;
}

template <>
float default_precision<float>() {
  return std::numeric_limits<float>::epsilon();
}

template <>
double default_precision<double>() {
  return std::numeric_limits<double>::epsilon();
}

  template <typename T>
  double range_size(const T& low, const T& high, const T& precision, bool exclude_low, bool exclude_high) {
    double result{static_cast<double>(high - low + 1)};
    if (exclude_low) {
      result -= 1.;
    }
    if (exclude_high) {
      result -= 1.;
    }
    return result;
  }

  template <>
  double range_size(const float& low, const float& high, const float& precision, bool exclude_low, bool exclude_high) {
    return high - low;
  }

  template <>
  double range_size(const double& low, const double& high, const double& precision, bool exclude_low, bool exclude_high) {
    return high - low;
  }

}
  
template <typename T>
class ParameterDomain {
 public:
  virtual ~ParameterDomain() = default;
  virtual bool in_domain(const T& value) const = 0;
  virtual double size() const = 0;
};

namespace impl {

template <typename T>
class CategoricalParameterDomain final : public ParameterDomain<T> {
 private:
  std::vector<T> values_;

 public:
  CategoricalParameterDomain(std::vector<T>&& values) : values_(std::move(values)) {}

  bool in_domain(const T& value) const override {
    for (const auto& v : values_) {
      if (value == v) {
        return true;
      }
    }
    return false;
  }

  double size() const override { return values_.size(); }
};

template <typename T>
class ContinuousParameterDomain final : public ParameterDomain<T> {
 private:
  const T low_;
  const T high_;
  const T precision_;
  const bool exclude_low_;
  const bool exclude_high_;

 public:
  ContinuousParameterDomain(T low, T high, T precision, bool exclude_low, bool exclude_high)
      : low_(low),
        high_(high),
        precision_(precision),
        exclude_low_(exclude_low),
        exclude_high_(exclude_high) {}

  bool in_domain(const T& value) const override {
    if (exclude_low_ && exclude_high_) {
      return low_ < value && value < high_;
    } else if (exclude_low_) {
      return low_ < value && value <= high_;
    } else if (exclude_high_) {
      return low_ <= value && value < high_;
    } else {
      return low_ <= value && value <= high_;
    }
  }

  double size() const override { return range_size(low_, high_, precision_, exclude_low_, exclude_high_); }
};

}  // namespace impl

template <typename T>
class CategoricalDomainBuilder final {
 private:
  std::vector<T> values_{};

 public:
  template <typename... Args>
  CategoricalDomainBuilder& with_values(Args... values) {
    values_.reserve(values_.size() + sizeof...(Args));
    (values_.emplace_back(std::forward<Args>(values)), ...);
    return *this;
  }

  std::unique_ptr<ParameterDomain<T>> create() {
    return std::make_unique<impl::CategoricalParameterDomain<T>>(std::move(values_));
  }
};

template <typename T>
class ContinuousDomainBuilder final {
 private:
  T low_{};
  T high_{};
  T precision_{impl::default_precision<T>()};
  bool exclude_low_{false};
  bool exclude_high_{false};

 public:
  ContinuousDomainBuilder& with_range(T low, T high) {
    low_ = low;
    high_ = high;
    return *this;
  }

  ContinuousDomainBuilder& with_precision(T precision) {
    precision_ = precision;
    return *this;
  }

  ContinuousDomainBuilder& exclude_low() {
    exclude_low_ = true;
    return *this;
  }

  ContinuousDomainBuilder& exclude_high() {
    exclude_high_ = true;
    return *this;
  }

  ContinuousDomainBuilder& include_low() {
    exclude_low_ = false;
    return *this;
  }

  ContinuousDomainBuilder& include_high() {
    exclude_high_ = false;
    return *this;
  }

  std::unique_ptr<ParameterDomain<T>> create() {
    return std::make_unique<impl::ContinuousParameterDomain<T>>(low_, high_, precision_,
                                                                exclude_low_, exclude_high_);
  }
};

template <typename T>
CategoricalDomainBuilder<T> configure_categorical_domain() {
  return CategoricalDomainBuilder<T>{};
}

template <typename T>
ContinuousDomainBuilder<T> configure_continuous_domain() {
  return ContinuousDomainBuilder<T>{};
}

}  // namespace tvsc::control

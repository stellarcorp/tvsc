#pragma once

#include <cstdint>
#include <random>

#include "hal2/peripheral.h"

namespace tvsc::hal2::random {

template <typename ResultType>
class Rng;

template <typename ResultType>
class RngPeripheral : public Peripheral<RngPeripheral<ResultType>, Rng<ResultType>> {
 public:
  using result_type = ResultType;

 private:
  // Turn on power and clock to this peripheral.
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  [[nodiscard]] virtual result_type generate() noexcept = 0;
  [[nodiscard]] constexpr double entropy() const noexcept {
    return std::numeric_limits<result_type>::digits;
  }

  friend class Rng<result_type>;

 public:
  virtual ~RngPeripheral() = default;
};

template <typename ResultType>
class Rng final : public Driver<RngPeripheral<ResultType>, Rng<ResultType>> {
 public:
  using result_type = ResultType;

 protected:
  explicit Rng(RngPeripheral<result_type>& peripheral) noexcept
      : Driver<RngPeripheral<result_type>, Rng<result_type>>(peripheral) {}

  friend class Peripheral<RngPeripheral<result_type>, Rng<result_type>>;

 public:
  [[nodiscard]] result_type generate() noexcept { return this->peripheral_->generate(); }
  [[nodiscard]] result_type operator()() noexcept { return this->peripheral_->generate(); }
  [[nodiscard]] constexpr double entropy() const noexcept { return this->peripheral_->entropy(); }

  [[nodiscard]] static constexpr result_type min() noexcept {
    return std::numeric_limits<result_type>::min();
  }

  [[nodiscard]] static constexpr result_type max() noexcept {
    return std::numeric_limits<result_type>::max();
  }
};
static_assert(
    std::uniform_random_bit_generator<Rng<uint32_t>>,
    "The Rng class is designed to satisfy std::uniform_random_bit_generator so that it can be used "
    "as a Generator type in various standard random number distributions like "
    "std::uniform_int_distribution and std::normal_distribution.");

}  // namespace tvsc::hal2::random

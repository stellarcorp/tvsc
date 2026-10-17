#pragma once

#include <concepts>
#include <cstddef>

namespace tvsc::hal2::config {

template <Facility F, size_t INDEX, typename T>
struct has_matching_accessor_name : std::false_type {};

template <typename T, Facility FACILITY, size_t INDEX>
concept ValidPeripheralAccessor =                              //
    has_matching_accessor_name<FACILITY, INDEX, T>::value and  //
    requires(T t) {
      { t.get() };
    } and  //
    true;

// Rules to enforce the naming conventions for each facility.
template <typename T>
struct has_matching_accessor_name<Facility::RCC, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template rcc() };
  };
};

/**
 * The primary oscillator for the system.
 */
template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator0() };
    { t.template primary_oscillator() };
  };
};

/**
 * This oscillator is contrived to be used during sleep and stop modes. In many architectures where
 * the watchdog uses a separate oscillator, this would be the one to use for the watchdog as well.
 */
template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator1() };
    { t.template low_power_oscillator() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator2() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::SYSTICK, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template systick() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio0() };
    { t.template gpio_a() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio1() };
    { t.template gpio_b() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio2() };
    { t.template gpio_c() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio3() };
    { t.template gpio_d() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 4, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio4() };
    { t.template gpio_e() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 5, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio5() };
    { t.template gpio_f() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 6, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio6() };
    { t.template gpio_g() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 7, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio7() };
    { t.template gpio_h() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::LED, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template debug_led() };
    { t.template led0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::LED, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template led1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::TIMER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::WATCHDOG, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template watchdog0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::WATCHDOG, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template watchdog1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::ADC, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template adc0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::ADC, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template adc1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::DAC, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template dac0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::DAC, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template dac1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::I2C, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::SPI, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::PROGRAMMER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template programmer0() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::IMU, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::RF_RECEIVER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_receiver0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::RF_RECEIVER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_receiver1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::RF_TRANSMITTER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_transmitter0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::RF_TRANSMITTER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_transmitter1() };
  };
};

}  // namespace tvsc::hal2::config

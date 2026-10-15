#pragma once

#include <concepts>
#include <cstddef>

namespace tvsc::hal::config {

template <Facility F, size_t INDEX>
struct FacilityAccessor;

template <Facility F, size_t INDEX, typename T>
struct has_matching_accessor_name : std::false_type {};

template <typename T>
concept ValidFacilityAccessor = requires {
  // Enforce that the type exposes the required static constants
  { T::facility() } -> std::same_as<Facility>;
  { T::index() } -> std::same_as<size_t>;

  // Extract them and pass them directly into the trait check
  requires has_matching_accessor_name<T::facility(), T::index(), T>::value;
};

template <typename T>
struct has_matching_accessor_name<Facility::RCC, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.rcc() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::SYSTICK, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.systick() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::POWER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.power() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.gpio0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.gpio1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::LED, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.debug_led() };
    { t.led0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::LED, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.led1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::TIMER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.timer0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.timer1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::WATCHDOG, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.watchdog0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::WATCHDOG, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.watchdog1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::ADC, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.adc0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::ADC, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.adc1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::DAC, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.dac0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::DAC, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.dac1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::I2C, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.i2c0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.i2c1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::SPI, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.spi0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.spi1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.can_bus0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.can_bus1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::PROGRAMMER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.programmer0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::PROGRAMMER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.programmer1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::IMU, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.imu0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.imu1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.power_monitor0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.power_monitor1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::RF_RECEIVER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.rf_receiver0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::RF_RECEIVER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.rf_receiver1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::RF_TRANSMITTER, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.rf_transmitter0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::RF_TRANSMITTER, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.rf_transmitter1() };
  };
};

}  // namespace tvsc::hal::config

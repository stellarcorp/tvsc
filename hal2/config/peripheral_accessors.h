#pragma once

#include <concepts>
#include <cstddef>

namespace tvsc::hal2::config {

template <Facility F, size_t INDEX, typename T>
struct has_matching_accessor_name : std::false_type {};

template <typename T, PeripheralId ID>
concept ValidPeripheralAccessor =                                    //
    has_matching_accessor_name<ID.facility, ID.index, T>::value and  //
    requires(T t) {
      { t.get() };
    } and  //
    true;

// Rules to enforce the naming conventions for each facility.
template <typename T>
struct has_matching_accessor_name<Facility::RCC_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template rcc() };
  };
};

/**
 * The primary oscillator for the system.
 *
 * On STM32 MCUs, this is usually the HSI (High-Speed Internal), MSI (Multi-Speed Internal), or HSE
 * (High-Speed External) oscillator.
 */
template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator0() };
    { t.template primary_oscillator() };
  };
};

/**
 * This oscillator is contrived to be used during sleep and stop modes. In many architectures where
 * the watchdog uses a separate oscillator, this would be the one to use for the watchdog as well.
 *
 * On STM32 MCUs, this is the LSI (Low-Speed Internal) or LSE (Low-Speed External).
 */
template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator1() };
    { t.template low_power_oscillator() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator2() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::OSCILLATOR_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template oscillator3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::SYSTICK_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template systick() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio0() };
    { t.template gpio_a() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio1() };
    { t.template gpio_b() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio2() };
    { t.template gpio_c() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio3() };
    { t.template gpio_d() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 4, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio4() };
    { t.template gpio_e() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 5, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio5() };
    { t.template gpio_f() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 6, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio6() };
    { t.template gpio_g() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::GPIO_FACILITY, 7, T> {
  static constexpr bool value = requires(T t) {
    { t.template gpio7() };
    { t.template gpio_h() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template lp_timer() };
    { t.template lp_timer0() };
    { t.template timer0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template lp_timer1() };
    { t.template timer1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer3() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 4, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer4() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 5, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer5() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 6, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer6() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::TIMER_FACILITY, 7, T> {
  static constexpr bool value = requires(T t) {
    { t.template timer7() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::CLOCK_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template steady_clock() };
    { t.template clock0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CLOCK_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template clock1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::WATCHDOG_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template watchdog0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::WATCHDOG_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template watchdog1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::ADC_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template adc0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::ADC_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template adc1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::DAC_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template dac0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::DAC_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template dac1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::I2C_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::I2C_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template i2c3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::SPI_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::SPI_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template spi3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::CAN_BUS_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template can_bus3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::LED_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template debug_led() };
    { t.template led0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::LED_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template led1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::LED_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template led2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::LED_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template led3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::PROGRAMMER_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template programmer0() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::IMU_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::IMU_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template imu3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor1() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR_FACILITY, 2, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor2() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::POWER_MONITOR_FACILITY, 3, T> {
  static constexpr bool value = requires(T t) {
    { t.template power_monitor3() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::RF_RECEIVER_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_receiver0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::RF_RECEIVER_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_receiver1() };
  };
};

template <typename T>
struct has_matching_accessor_name<Facility::RF_TRANSMITTER_FACILITY, 0, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_transmitter0() };
  };
};
template <typename T>
struct has_matching_accessor_name<Facility::RF_TRANSMITTER_FACILITY, 1, T> {
  static constexpr bool value = requires(T t) {
    { t.template rf_transmitter1() };
  };
};

}  // namespace tvsc::hal2::config

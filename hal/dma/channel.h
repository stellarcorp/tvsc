#pragma once

#include <concepts>
#include <utility>

#include "hal/mcu/mcu.h"

namespace tvsc::hal::dma {

template <typename T>
concept ValidDmaFacility = requires(tvsc::hal::mcu::Mcu& mcu) {
  { T::handle_interrupt(mcu) } -> std::same_as<void>;
  { T::preempt_priority() } -> std::convertible_to<uint32_t>;
  { T::sub_priority() } -> std::convertible_to<uint32_t>;
  { T::facility_type() } -> std::same_as<mcu::DmaFacility>;
};

struct NullFacility final {
  static constexpr uint32_t preempt_priority() noexcept { return 0; }
  static constexpr uint32_t sub_priority() noexcept { return 0; }
  static constexpr mcu::DmaFacility facility_type() noexcept {
    return mcu::DmaFacility::FACILITY_NONE;
  }

  __attribute__((always_inline)) static void handle_interrupt(tvsc::hal::mcu::Mcu& mcu) {
    __builtin_trap();
  }
};
static_assert(ValidDmaFacility<NullFacility>);

template <size_t INDEX, uint32_t PREEMPT_PRIORITY = 2, uint32_t SUB_PRIORITY = 0>
struct AdcFacility final {
  static constexpr uint32_t preempt_priority() noexcept { return PREEMPT_PRIORITY; }
  static constexpr uint32_t sub_priority() noexcept { return SUB_PRIORITY; }
  static constexpr mcu::DmaFacility facility_type() noexcept {
    static_assert(INDEX <= 2, "Need to extend facility_type() in AdcFacility class.");
    if constexpr (INDEX == 0) {
      return mcu::DmaFacility::FACILITY_ADC1;
    } else if constexpr (INDEX == 1) {
      return mcu::DmaFacility::FACILITY_ADC2;
    } else if constexpr (INDEX == 2) {
      return mcu::DmaFacility::FACILITY_ADC3;
    } else {
#if defined(__cpp_lib_unreachable) && (__cpp_lib_unreachable >= 202202L)
      std::unreachable();
#else
      __builtin_unreachable();
#endif
    }
  }

  __attribute__((always_inline)) static void handle_interrupt(tvsc::hal::mcu::Mcu& mcu) {
    mcu.adc<INDEX>().handle_interrupt();
  }
};
static_assert(ValidDmaFacility<AdcFacility<0>>);

template <size_t INDEX, uint32_t PREEMPT_PRIORITY = 2, uint32_t SUB_PRIORITY = 0>
struct DacFacility final {
  static constexpr uint32_t preempt_priority() noexcept {
#if defined(__cpp_lib_unreachable) && (__cpp_lib_unreachable >= 202202L)
    std::unreachable();
#else
    __builtin_unreachable();
#endif
  }
  static constexpr uint32_t sub_priority() noexcept { return SUB_PRIORITY; }
  static constexpr mcu::DmaFacility facility_type() noexcept {
    static_assert(INDEX <= 1, "Need to extend facility_type() in DacFacility class.");
    if constexpr (INDEX == 0) {
      return mcu::DmaFacility::FACILITY_DAC1_CH1;
    } else if constexpr (INDEX == 1) {
      return mcu::DmaFacility::FACILITY_DAC1_CH2;
    } else {
#if defined(__cpp_lib_unreachable) && (__cpp_lib_unreachable >= 202202L)
      std::unreachable();
#else
      __builtin_unreachable();
#endif
    }
  }

  __attribute__((always_inline)) static void handle_interrupt(tvsc::hal::mcu::Mcu& mcu) {
    mcu.dac<INDEX>().handle_interrupt();
  }
};
static_assert(ValidDmaFacility<DacFacility<0>>);

template <ValidDmaFacility Facility>
__attribute__((always_inline)) inline void dispatch_dma_interrupt() {
  Facility::handle_interrupt(tvsc::hal::mcu::Mcu::mcu());
}

template <typename T>
concept ValidDmaConfiguration = requires {
  typename T::Dma1Channel1Facility;
  typename T::Dma1Channel2Facility;
  typename T::Dma1Channel3Facility;
  typename T::Dma1Channel4Facility;
  typename T::Dma1Channel5Facility;
  typename T::Dma1Channel6Facility;
  typename T::Dma1Channel7Facility;
  typename T::Dma2Channel1Facility;
  typename T::Dma2Channel2Facility;
  typename T::Dma2Channel3Facility;
  typename T::Dma2Channel4Facility;
  typename T::Dma2Channel5Facility;
  typename T::Dma2Channel6Facility;
  typename T::Dma2Channel7Facility;
  requires ValidDmaFacility<typename T::Dma1Channel1Facility>;
  requires ValidDmaFacility<typename T::Dma1Channel2Facility>;
  requires ValidDmaFacility<typename T::Dma1Channel3Facility>;
  requires ValidDmaFacility<typename T::Dma1Channel4Facility>;
  requires ValidDmaFacility<typename T::Dma1Channel5Facility>;
  requires ValidDmaFacility<typename T::Dma1Channel6Facility>;
  requires ValidDmaFacility<typename T::Dma1Channel7Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel1Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel2Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel3Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel4Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel5Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel6Facility>;
  requires ValidDmaFacility<typename T::Dma2Channel7Facility>;
};

struct DefaultDmaConfiguration {
  using Dma1Channel1Facility = NullFacility;
  using Dma1Channel2Facility = NullFacility;
  using Dma1Channel3Facility = NullFacility;
  using Dma1Channel4Facility = NullFacility;
  using Dma1Channel5Facility = NullFacility;
  using Dma1Channel6Facility = NullFacility;
  using Dma1Channel7Facility = NullFacility;

  using Dma2Channel1Facility = NullFacility;
  using Dma2Channel2Facility = NullFacility;
  using Dma2Channel3Facility = NullFacility;
  using Dma2Channel4Facility = NullFacility;
  using Dma2Channel5Facility = NullFacility;
  using Dma2Channel6Facility = NullFacility;
  using Dma2Channel7Facility = NullFacility;

 private:
  DefaultDmaConfiguration() = default;
};
static_assert(ValidDmaConfiguration<DefaultDmaConfiguration>);

template <ValidDmaConfiguration Config>
void initialize_dma_interrupts() {
  if constexpr (!std::is_same_v<typename Config::Dma1Channel1Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel1Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 1, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 1) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma1Channel2Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel2Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 2, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 2) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma1Channel3Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel3Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 3, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 3) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma1Channel4Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel4Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 4, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 4) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma1Channel5Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel5Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 5, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 5) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma1Channel6Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel6Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 6, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 6) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma1Channel7Facility, NullFacility>) {
    using Facility = typename Config::Dma1Channel7Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(1, 7, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 1, Channel 7) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel1Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel1Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 1, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 1) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel2Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel2Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 2, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 2) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel3Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel3Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 3, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 3) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel4Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel4Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 4, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 4) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel4_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel4_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel5Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel5Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 5, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 5) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel5_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel5_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel6Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel6Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 6, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 6) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel6_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel6_IRQn);
  }

  if constexpr (!std::is_same_v<typename Config::Dma2Channel7Facility, NullFacility>) {
    using Facility = typename Config::Dma2Channel7Facility;
    static_assert(
        tvsc::hal::mcu::Mcu::is_facility_valid_for_dma_channel(2, 7, Facility::facility_type()),
        "ERROR: The assigned facility is not valid on the given DMA channel (DMA 2, Channel 7) "
        "this MCU.");

    HAL_NVIC_SetPriority(DMA2_Channel7_IRQn, Facility::preempt_priority(),
                         Facility::sub_priority());
    HAL_NVIC_EnableIRQ(DMA2_Channel7_IRQn);
  }
}

}  // namespace tvsc::hal::dma

#define DEFINE_DMA_IRQ_HANDLERS(ConfigType)                                              \
  extern "C" {                                                                           \
  void DMA1_Channel1_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel1Facility>(); \
  }                                                                                      \
  void DMA1_Channel2_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel2Facility>(); \
  }                                                                                      \
  void DMA1_Channel3_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel3Facility>(); \
  }                                                                                      \
  void DMA1_Channel4_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel4Facility>(); \
  }                                                                                      \
  void DMA1_Channel5_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel5Facility>(); \
  }                                                                                      \
  void DMA1_Channel6_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel6Facility>(); \
  }                                                                                      \
  void DMA1_Channel7_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma1Channel7Facility>(); \
  }                                                                                      \
  void DMA2_Channel1_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel1Facility>(); \
  }                                                                                      \
  void DMA2_Channel2_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel2Facility>(); \
  }                                                                                      \
  void DMA2_Channel3_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel3Facility>(); \
  }                                                                                      \
  void DMA2_Channel4_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel4Facility>(); \
  }                                                                                      \
  void DMA2_Channel5_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel5Facility>(); \
  }                                                                                      \
  void DMA2_Channel6_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel6Facility>(); \
  }                                                                                      \
  void DMA2_Channel7_IRQHandler() {                                                      \
    tvsc::hal::dma::dispatch_dma_interrupt<typename ConfigType::Dma2Channel7Facility>(); \
  }                                                                                      \
  }

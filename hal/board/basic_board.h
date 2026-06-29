#pragma once

#include <concepts>
#include <type_traits>

#include "hal/gpio/gpio.h"
#include "hal/mcu/mcu.h"

namespace tvsc::hal::board {

template <typename B, std::size_t MINIMUM_COUNT = 1>
concept HasDebugLed =                         //
    (MINIMUM_COUNT > 0) and                   //
    requires(B& b) {
      { b.template debug_led<0>() } -> std::same_as<gpio::PinPeripheral&>;
      { b.template debug_led<MINIMUM_COUNT - 1>() } -> std::same_as<gpio::PinPeripheral&>;
    } and  //
    true;

template <typename B>
concept HasMcu =  //
    requires(B& b) {
      { b.mcu() } -> std::same_as<mcu::Mcu&>;
    } and  //
    true;

template <typename B>
concept BasicBoard =    //
    HasDebugLed<B> and  //
    HasMcu<B> and       //
    true;

}  // namespace tvsc::hal::board

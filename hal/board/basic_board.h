#pragma once

#include <concepts>
#include <type_traits>

#include "hal/gpio/gpio.h"
#include "hal/power/power.h"
#include "hal/rcc/rcc.h"
#include "hal/systick/systick.h"

namespace tvsc::hal::board {

template <typename B, std::size_t MINIMUM_COUNT = 1>
concept HasDebugLed =                         //
    (MINIMUM_COUNT > 0) and                   //
    (B::NUM_DEBUG_LEDS >= MINIMUM_COUNT) and  //
    requires(B& b) {
      { b.template debug_led<0>() } -> std::same_as<gpio::PinPeripheral&>;
      { b.template debug_led<MINIMUM_COUNT - 1>() } -> std::same_as<gpio::PinPeripheral&>;
    } and  //
    true;

template <typename B>
concept HasRcc =  //
    requires(B& b) {
      { b.rcc() } -> std::same_as<rcc::Rcc&>;
    } and  //
    true;

template <typename B>
concept HasPower =  //
    requires(B& b) {
      { b.power() } -> std::same_as<power::Power&>;
    } and  //
    true;

template <typename B>
concept HasSysTick =  //
    requires(B& b) {
      { b.sys_tick() } -> std::same_as<systick::SysTickType&>;
    } and  //
    true;

template <typename B>
concept BasicBoard =    //
    HasDebugLed<B> and  //
    HasRcc<B> and       //
    HasPower<B> and     //
    HasSysTick<B> and   //
    true;

}  // namespace tvsc::hal::board

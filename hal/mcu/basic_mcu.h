#pragma once

#include <concepts>
#include <type_traits>

#include "hal/gpio/gpio.h"
#include "hal/power/power.h"
#include "hal/rcc/rcc.h"
#include "hal/systick/systick.h"

namespace tvsc::hal::mcu {

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
concept HasCreatePeripheral =  //
    requires(B& b, gpio::PinRef ref) {
      { b.create_peripheral(ref) } -> std::same_as<gpio::PinPeripheral>;
    } and  //
    true;

template <typename B>
concept BasicMcu =              //
    HasRcc<B> and               //
    HasPower<B> and             //
    HasSysTick<B> and           //
    HasCreatePeripheral<B> and  //
    true;

}  // namespace tvsc::hal::mcu

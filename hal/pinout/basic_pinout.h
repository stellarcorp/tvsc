#pragma once

#include <array>
#include <concepts>
#include <type_traits>

#include "hal/gpio/gpio.h"

namespace tvsc::hal::pinout {

template <typename P, std::size_t MINIMUM_COUNT = 1>
concept HasLeds =                                                                                 //
    (MINIMUM_COUNT > 0) and                                                                       //
    (P::NUM_LEDS >= MINIMUM_COUNT) and                                                            //
    requires { std::same_as<decltype(P::LED_PINS), std::array<gpio::PinRef, P::NUM_LEDS>>; } and  //
    true;

template <typename P>
concept BasicPinout =  //
    HasLeds<P> and     //
    true;

}  // namespace tvsc::hal::pinout

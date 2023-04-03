#pragma once

namespace tvsc::bus::gpio {

/**
 * Mapping of functions to pin numbers. This mapping may be the physical pin, a GPIO or WiringPi
 * designator, or whatever semantic is needed for the given board.
 *
 * The approach here is to fix the interface -- which functions need pins -- but link against
 * different implementations of this class based on the board we are using.
 *
 * The upside to this approach is that we fully fix the mapping via function names and get compile
 * errors if the implementations, the interface, and the usage are not all in sync. The downside is
 * that this results in a function call every time we want to get a pin number. That function call
 * is irrelevant in most cases, but if we find ourselves calling these functions often or in a loop,
 * we should consider other designs.
 *
 * Another approach would be to use a class templated on something benign, such as the application
 * name and/or the board name. Specializations would then be in separate header files, and we would
 * have conditional includes in a single header file for that application's pin mapping. Each
 * function could then be "static constexpr". This would avoid the overhead of the function calls by
 * allowing the compiler to inline those calls, giving performance and effect similar to individual
 * #define'd symbols. But this approach would require us to manage compilation symbols and
 * conditional compilation.
 */
class BlinkPinMapping final {
 public:
  static int led_pin();
};

}  // namespace tvsc::bus::gpio

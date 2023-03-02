# [EpoxyDuino](https://github.com/bxparks/EpoxyDuino)

This package provides a partial implementation of the [ArduinoCore-API](https://github.com/arduino/ArduinoCore-API) to enable compiling Arduino code on non-Arduino platforms, such as Linux desktops. EpoxyDuino is sufficient for now, but it is incomplete and inaccurate. It does not implement the full Arduino API, and its types are often incompatible with the types from the API. It seems focused on AVRs and the ESP8266 platforms, and it ignores other Arduino-compatible platforms.

## Patches

### attachInterrupt(), detachInterrupt()

These functions were declared, but not defined. Also, the declarations used `int` instead of `PinStatus` for the mode.

We gave empty definitions for these functions. That works for trivial tests but will likely be insufficient even in the near term.

### PinStatus and PinMode

These enum types were undefined. Some of symbols were `#define`'d as ints with compatible values, but many of the symbols were undefined.

We added these enums under condition compilation.


## Longer term plan

We will need to pull in the ArduinoCore-API package and implement it for Debian Linux, both x86_64 and arm64. This work is required to get the RF69 drivers from the RadioHead library working on the Odroid M1 and Raspberry Pi boards. We may also need to provide an implementation that imitates the Teensy 4.x boards to allow for higher fidelity testing of radio modules on that platform.

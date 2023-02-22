# Radio

This package contains low-level configuration, communications and status APIs for working with radio modules. Radio modules, in this sense, tend to be RF daughterboards or ICs that have interfaces over CAN bus, SPI, UART, etc.

## Settings

The settings API contains generic types for exchanging and persisting configuration information. It is based on protocol buffers with special settings and libraries to use them on x86- and ARM-based Linux systems, as well as Arduino-compatible embedded devices.

## Communications

The communications API contains simple send/receive functions along with other associated functions to manage the communications state.

## Status

The status API gives a set of measured values that can be published via a telemetry package (to be written).

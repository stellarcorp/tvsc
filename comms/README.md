# Comms

This package contains the core communications modules. It covers low-level radio devices, packet networking, and specific protocols, such as TDMA.

## Subpackages

* [radio](./radio) for controlling low-level radio modules.
* [packet](./packet) for packet queuing and processing, including disassociation into transmittable fragments.
* [tdma](./tdma) for an implementation of [TDMA](https://en.wikipedia.org/wiki/Time-division_multiple_access) 
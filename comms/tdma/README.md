# TDMA communications

This package contains the transceiver and other abstractions for TDMA communications. It is designed as a Dynamic TDMA implementation, but can also be used without the dynamic aspect by simply giving each node the same amount of transmission time.

The primary class in this package is TdmaTransceiver. It manages time slots on top of any half-duplex radio.

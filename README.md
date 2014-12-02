libfonz
=======

Fonz Packet Processing library. A mechanism for cheaply and reliably
communicating between two simple systems.

Packets are comprised of a world-unique header (0xce) one or three bytes
of payload, and a checksum. The minimum number of packet bytes is 3 and
the maximum is 8.

There are two reserved bytes. 0xce represents a header. If it is seen in
a data stream, then the state is back to SAWHDR, regardless of anything
else. No exceptions. The other reserved byte is 0xcc, which is an escape
byte. If seen, then the next byte will have its high bit turned on. So
if the payload contains either 0xce or 0xcc, then {0xcc, 0x4e} or {0xcc,
0x4c} is sent instead.

The library has been implemented twice. Firstly, as a lightweight version
for the Atmel AVR (or Arduino) and secondly as a main, prioritised
library for systems with more memory and bigger processors.

Normally, you'd run the AVR mini library on your embedded system, and
the main library on the machine that's talking to it.

AVR implementation of libfonz
=============================

This particular implementation of the library is designed to be used on
embedded systems, such as the Atmel AVR. It includes functions for
handling SIO interrupts, enqueuing and dequeing packets, and basic
packet allocation.

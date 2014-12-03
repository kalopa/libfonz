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

Installing
==========

```bash
$ git clone https://github.com/kalopa/libfonz
$ cd libfonz
$ make
```

### AVR implementation of libfonz

This particular implementation of the library is designed to be used on
embedded systems, such as the Atmel AVR. It includes functions for
handling SIO interrupts, enqueuing and dequeing packets, and basic
packet allocation.

```C
;
; Interrupt Vector table.
	.arch	atmega8
	.section .vectors,"ax",@progbits
	.global	__vectors
	.func	__vectors
__vectors:
	rjmp	_reset		; Main reset
	rjmp	nointr		; External Interrupt 0
	rjmp	nointr		; External Interrupt 1
	rjmp	nointr		; Timer 2 Compare
	rjmp	nointr		; Timer 2 Overflow
	rjmp	nointr		; Timer 1 Capture Event
	rjmp	nointr		; Timer 1 Compare Match A
	rjmp	nointr		; Timer 1 Compare Match B
	rjmp	nointr		; Timer 1 Counter Overflow
	rjmp	nointr		; Timer 0 Counter Overflow
	rjmp	nointr		; SPI Serial Transfer Complete
	rjmp	_fonz_in	; USART Rx Complete
	rjmp	_fonz_out	; USART Data Register Empty
	rjmp	nointr		; USART Tx Complete
	rjmp	nointr		; ADC Conversion Complete
	rjmp	nointr		; EEPROM Ready
	rjmp	nointr		; Analog Comparator
	rjmp	nointr		; Two Wire Interface
	rjmp	nointr		; Store Program Memory Ready
```

(redirect the `RXC` and `UDRE` interrupt vectors to `_fonz_in()` and `_fonz_out()` respectively).

In your code, include the header file and call the `fp_init()` function.

```C
#include "libfonz.h"

main()
{
	struct fonz *fp;

	...
	fp_init(8, 8);
	...
	sei();
	...
	while (1) {
		if ((fp = fp_receive()) != NULL) {
			/*
			 * Process received packet.
			 */
			...
			fp_free(fp);
		}
	}
	...
}
```

Packet Operation
================

The `fp_init()` function pre-allocates packets for the send and receive queues.
The queues are separate so that synchronous messages can be sent and received
without the sender exhausting the free packet pool, and being unable to receive
a response.

The `fp_receive()` function will attempt to remove a packet from the receive
queue, returning NULL if it is empty.
On completion of packet processing, call `fp_free()` to return the packet to
the receive packet pool.

The `fonz` struct has the following data types:

```C
struct  fonz    {
	struct	fonz	*fp_next;
	unsigned char	fp_cmd;
	unsigned char	fp_arg1;
	unsigned char	fp_arg2;
};
```

Commands with the least-significant bit set are assumed to have two single-byte arguments,
`fp_arg1` and `fp_arg2`.
All other commands do not have arguments.

Think of these as GET/SET pairs, which you can define.
For example, `0x70` might be defined as "Get Time" and `0x71` as "Set Time."
In a client/server application, the client may use the Get Time command and then respond
to a Set Time packet.

The protocol is asynchronous, so a Set Time response to a Get Time packet is possible but
must be coded that way.
In other words, there is no defined response to any given command.

*Do not* re-use a received packet and enqueue it as a response for transmission
back to the sender.
The receive and send packet pools are isolated, and this will cause the receiver
to run out of free packets.

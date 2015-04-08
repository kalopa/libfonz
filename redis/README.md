Redis interface
===============

The fonz library can't really handle concurrency in any meaningful
way. If you have a dumb device at the end of a serial line, and lots of
clients which are trying to connect to the serial line and send commands,
things will just break. The complex solution is to use the (as yet
unfinished) fonz switch.

The easiest solution is to use this Redis interface library, assuming you
have Redis installed. It uses the Redis pub/sub mechanism to communicate
with the device. In operation, the daemon opens the serial port and
forks. One process subscribes to the "fonz-out" queue and parses the
JSON messages sent to it, copying them to the serial port. The other half
of the daemon tries to read from the serial port, and takes whatever it
receives and publishes it to the "fonz-in" queue. To send a fonz packet to
the device, publish a message to "fonz-out." If you're interested in what
comes back from the device, subscribe to the "fonz-in" queue. Eventually
this thing will be smarter in terms of subscribing to certain message
types. Either that, or I'll finish the fonz-switch daemon which will
operate independent of Redis and will be much smarter about subscriptions.

### AVR library for libfonz

The AVR version of libfonz is slightly different to the generic
version. Mostly this is due to size limitations and optimizations. The
main difference is that the src and dst values are not sent or
received. Use the fonz packet switch for normal communications between
AVR devices and systems running the generic library. The packet switch
also handles any race condition issues between multiple senders and
receivers. See the switch README for more details.

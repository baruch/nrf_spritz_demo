NRF24LE1 Spritz demo
==================

This is a demo program to show the use of an nRF24LE1 with Spritz, an RC4 replacement from Ron Rivest.

Original Spritz code is from https://github.com/jedisct1/spritz
It was released under a public domain declaration.

The main advantage of Spritz over RC4 is that the same construction also
provides a MAC that can be used to authenticate RF messages without them being
modifiable like CRC allows with a stream cipher.

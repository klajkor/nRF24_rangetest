# nRF24 rangetest
Combined code for TX and RX node, see source code for instructions

* This code is based on the work of iforce2d.
* Link to his original code: http://www.iforce2d.net/sketches/nRFRangeTest30km.zip
* Transmitter: just sends 12-byte packet continously
* Receiver: display the statistics
* nRF24L01 library: https://github.com/maniacbug/RF24

## nRF24L01 connections 
* 1 - GND
* 2 - VCC 3.3V !!! Ideally 3.0v, definitely not 5V
* 3 - CE to Arduino pin 9
* 4 - CSN to Arduino pin 10
* 5 - SCK to Arduino pin 13
* 6 - MOSI to Arduino pin 11
* 7 - MISO to Arduino pin 12
* 8 - UNUSED
 
## Wiring, Arduino Uno/Nano
### SPI
* MOSI: 11
* MISO: 12
* SCK: 13
* CE: 9
* CSN: 10

### I2C
* SDA: A4
* SCL: A5


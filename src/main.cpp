/*
Simple transmitter, just sends 12-byte packet continously
nRF24L01 library: https://github.com/gcopeland/RF24

nRF24L01 connections 
 1 - GND
 2 - VCC 3.3V !!! Ideally 3.0v, definitely not 5V
 3 - CE to Arduino pin 9
 4 - CSN to Arduino pin 10
 5 - SCK to Arduino pin 13
 6 - MOSI to Arduino pin 11
 7 - MISO to Arduino pin 12
 8 - UNUSED
 
*/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "U8glib.h"

/* Uncomment 
* - either TX_NODE for building Transmit Node
* - o RX_NODE for building Receive Node
* but ONLY one of them!
*/
#define TX_NODE 1
//#define RX_NODE 1

#define AVG_SECONDS 10

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);	// Fast I2C / TWI

RF24 radio(9, 10);

const uint64_t pipe_address =  0xE8E8F0F0E1LL;

// The sizeof this struct should not exceed 32 bytes
struct PacketData {
  unsigned long hours;
  unsigned long minutes;
  unsigned long seconds;
};

PacketData data;

unsigned long lastTick = 0;
int packetCounts[10];
int packetCountIndex = 0;
int packetCountTotal = 0;

int avgs[AVG_SECONDS];
int avgIndex = 0;
unsigned long avgTotal = 0;

unsigned long packetsRead = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastAvgUpdate = 0;
unsigned long lastRecvTime = 0;
unsigned long drops = 0;

char ppsBuf[16];
char avgBuf[16];
char hmsBuf[16];

void Radio_Init(void);
void Oled_Init(void);
void TX_test(void);
void RX_test(void);
void recvData(void);
void draw_screen(void);
void updateScreen(void);

void setup() {
  
  Serial.begin(115200);
  #ifdef RX_NODE
    Oled_Init();
  #endif
  Radio_Init();  
  // Data initialization
  memset(&data, 0, sizeof(PacketData));  
  memset( packetCounts, 0, sizeof(packetCounts) );
  memset( avgs, 0, sizeof(avgs) );
}

void Oled_Init(void)
{
  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_fur11);
}

void Radio_Init(void)
{
  int role=0;
  while(radio.begin()==false)
  {
    Serial.println(F("Radio begin failed"));    
    delay(2000);
  }
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  #ifdef TX_NODE
    role++;
    radio.openWritingPipe(pipe_address);    
  #endif
  #ifdef RX_NODE
    role++;
    radio.openReadingPipe(1,pipe_address);
    radio.startListening();
  #endif
  if (role!=1)
  {
    Serial.println(F("Wrong TX/RX config, correct it. Program halted"));
    while(1) 
    {
      delay(1000);
    }
  }
}


void loop() {
  #ifdef TX_NODE
    TX_test();    
  #endif
  #ifdef RX_NODE
    RX_test();    
  #endif
}

/* Code below is for Transmit Node only */
#ifdef TX_NODE
void TX_test(void)
{
  unsigned long now = millis();
  if ( now - lastTick >= 1000 ) {
    data.seconds++;
    if ( data.seconds >= 60 ) {
      data.seconds = 0;
      data.minutes++;
    }
    if ( data.minutes >= 60 ) {
      data.minutes = 0;
      data.hours++;
    }
    lastTick = now;
  }
    
  radio.write(&data, sizeof(PacketData));    
}
#endif // TX_NODE

/* Code below is for Receive Node only */
#ifdef RX_NODE
void recvData(void)
{  
  while ( radio.available() ) {        
    radio.read(&data, sizeof(PacketData));
    packetsRead++;
    lastRecvTime = millis();
  }
}

void draw_screen(void) {
  u8g.drawStr( 2, 24, ppsBuf);
  u8g.drawStr( 2, 40, avgBuf);
  u8g.drawStr( 2, 56, hmsBuf);
}

void updateScreen(void)
{  
  unsigned long now = millis();
  if ( now - lastScreenUpdate < 100 )
    return;
    
  // moving average over 1 second
  packetCountTotal -= packetCounts[packetCountIndex];
  packetCounts[packetCountIndex] = packetsRead;
  packetCountTotal += packetsRead;

  packetCountIndex = (packetCountIndex + 1) % 10;
  packetsRead = 0;
  
  sprintf(ppsBuf, "PPS: %d", packetCountTotal);
  sprintf(avgBuf, "AVG: %ld", avgTotal / AVG_SECONDS);
  sprintf(hmsBuf, "%02ld:%02ld:%02ld", data.hours, data.minutes, data.seconds);

  u8g.firstPage();
  do {
    draw_screen();
  } 
  while( u8g.nextPage() );
  
  lastScreenUpdate = millis();

  if ( now - lastAvgUpdate >= 1000 ) {    
    // moving average of 1 second moving average
    avgTotal -= avgs[avgIndex];
    avgs[avgIndex] = packetCountTotal;
    avgTotal += packetCountTotal;
  
    avgIndex = (avgIndex + 1) % AVG_SECONDS;
    lastAvgUpdate = millis();
  }
}

void RX_test(void)
{
  recvData();
  updateScreen();
}
#endif //RX_NODE
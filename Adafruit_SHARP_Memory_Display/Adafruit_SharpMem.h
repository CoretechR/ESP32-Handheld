/*********************************************************************
This is an Arduino library for our Monochrome SHARP Memory Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

These displays use SPI to communicate, 3 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#define BLACK 0
#define WHITE 1

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#if defined(RAMSTART) && defined(RAMEND) && ((RAMEND-RAMSTART) < 4096)
  #warning "Display may not work on devices with less than 4K RAM"
#endif
 
#include <Adafruit_GFX.h>
#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif

#include <SPI.h>

#if defined(ARDUINO_STM32_FEATHER)
  typedef volatile uint32 RwReg;
  //#define USE_FAST_PINIO
#elif defined(ARDUINO_FEATHER52) || defined (ESP8266) || defined (ESP32) || defined(__SAM3X8E__) || defined(ARDUINO_ARCH_SAMD)
  typedef volatile uint32_t RwReg;
  #define USE_FAST_PINIO  // tested!
#elif defined (__AVR__) || defined(TEENSYDUINO)
  typedef volatile uint8_t RwReg;
  #define USE_FAST_PINIO
#else
  #undef USE_FAST_PINIO  
#endif


class Adafruit_SharpMem : public Adafruit_GFX {
 public:
  Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss, uint16_t w = 96, uint16_t h = 96);
  boolean begin();
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getPixel(uint16_t x, uint16_t y);
  void clearDisplay();
  void fillScreen(uint16_t color);
  void refresh(void);

 private:
  uint8_t _ss, _clk, _mosi;
  uint32_t _sharpmem_vcom;

#ifdef USE_FAST_PINIO
  volatile RwReg *dataport, *clkport;
  uint32_t datapinmask, clkpinmask;
#endif
  
  void sendbyte(uint8_t data);
  void sendbyteLSB(uint8_t data);
};

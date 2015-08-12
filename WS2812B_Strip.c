/*
 * WS2812BStrip.cpp
 *
 *  Created on: Apr 1, 2015
 *      Author: TWeaK
 */

#include <msp430f2272.h>
#include "WS2812B_Strip.h"
#include "main.h"

// "Constructor"
void create(struct WS2812B_Strip *strip, const uint16_t numberOfPixels)
{
  strip->numberOfPixels = numberOfPixels;
  strip->numberOfBytes = 3 * numberOfPixels;
  strip->brightness = 0;
  strip->inShow = false;
  strip->inInterrupt = false;
  strip->breakFromPattern = false;

  // Blank our pixel memory array.
  uint16_t i;
  for(i = 0; i < strip->numberOfBytes; i++)
  {
	strip->pixels[i] = 0;
  }
}


// Sets the data for out entire pixel array to zeros (blank.)
void clear(struct WS2812B_Strip *strip)
{
  // Clear our pixel array.
  uint16_t i;
  for(i = 0; i < strip->numberOfBytes; i++)
  {
	strip->pixels[i] = 0;
  }
}


// 62.5ns per instruction.
//
// 1.25us ideal per "bit" = 20 instructions per "bit"
// 1.25us (+/- 300ns => 950ns - 1.55ns)
// 800us high, 450us low = ZERO  => 13 cycles high, 7 cycles low
// 450us high, 800us low = ONE  =>  7 cycles high, 13 cycles low
//
// NOTES: BIT0 runs slightly slower than the Bits: BIT7 - BIT1 because of the
//  pointer increment and the while loop test.
//
// TIMING MEASUREMENTS:
//      tH ONE: 725ns   (nominal 800ns  +/- 150ns) OK!!! (-75ns off)
//      tL ONE: 715ns   (nominal 450ns  +/- 150ns) OK... (+265ns off)
//  period ONE: 1440ns  (nominal 1250ns +/- 600ns) OK!!! (+190ns off)
//
//     tH ZERO: 390ns   (nominal 400ns  +/- 150ns) OK!!! (-10ns off)
//     tL ZERO: 790ns   (nominal 850ns  +/- 150ns) OK!!! (-60ns off)
// period ZERO: 1180ns  (nominal 1250ns +/- 600ns) OK!!! (-70ns off)
//
//     tH LAST ONE: 725ns   (nominal 800ns  +/- 150ns) OK!!! (-75ns off)
//     tL LAST ONE: 985ns   (nominal 450ns  +/- 150ns) Uhh   (+535ns off)
// period LAST ONE: 1550ns  (nominal 1250ns +/- 600ns) ok!!! (+460ns off)
//
//     tH LAST ZERO: 460ns   (nominal 400ns  +/- 150ns) OK!!! (+60ns off)
//     tL LAST ZERO: 1100ns  (nominal 850ns  +/- 150ns) OK    (+250ns off)
// period LAST ZERO: 1560ns  (nominal 1250ns +/- 600ns) OK!!! (+310ns off)
//
//
// PSUEDO:
//  1. Turn off Interrupts!  Time critical.
//  2. 100us pause to reset the data cycle.
//  3. get end address of pixels[] (pixels + numberOfBytes)
//  4. Get pointer for each byte to be written (bit by bit).
//  5. Write each byte (bit by bit) for "numberOfBytes".
//  6. increment pointer.
//  7. All Data written. Turn on interrups back on.
//----------------------------------------------------------------------------
void show(struct WS2812B_Strip *strip)
{
  if (strip->inShow == true)
	return;

  //  1. Turn off Interrupts!  Time critical.
  // DISABLE global interrupts.  "Bit Clear Status Register"
  __bic_SR_register(GIE);
  strip->inShow = true;

  //  2. Turn output low for 50us. (800 cycles @16MHz ~= 50us).
  SERIAL_OUTPUT_PORT &= ~SERIAL_OUTPUT_PIN;
  __delay_cycles(800);

  //  3. get end address of pixels[] (pixels + numberOfBytes)
  //  4. Get pointer for each byte to be written (bit by bit).
  uint16_t  bytesLeft = strip->numberOfBytes;
  uint8_t *ptr = strip->pixels;                      // Pointer to the current byte we are writing.

  //  5. Write each byte (bit by bit) for "numberOfBytes".
  while(bytesLeft != 0)  // 2 cycles.
  {
    if (*ptr & BIT7)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT6)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT5)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT4)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT3)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT2)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT1)
      writeOne();
    else
      writeZero();

    if (*ptr & BIT0)
    {
      // Write last One
      SERIAL_OUTPUT_PORT |= SERIAL_OUTPUT_PIN;
      bytesLeft--;
      ptr++;
      _no_operation();
      _no_operation();
      _no_operation();
      _no_operation();

      SERIAL_OUTPUT_PORT &= ~SERIAL_OUTPUT_PIN;
    }
    else
    {
      // Write last Zero
      SERIAL_OUTPUT_PORT |= SERIAL_OUTPUT_PIN;
      bytesLeft--;
      ptr++;

      SERIAL_OUTPUT_PORT &= ~SERIAL_OUTPUT_PIN;
    }

  }


  //  7. All Data written. Turn on interrups back on.
  __bis_SR_register(GIE);    // Enable global interrupts.  "Bit Set Status Register"
  strip->inShow = false;
}

inline void writeOne(void)
{
  SERIAL_OUTPUT_PORT |= SERIAL_OUTPUT_PIN;
  _no_operation();
  _no_operation();
  _no_operation();
  _no_operation();
  _no_operation();
  _no_operation();

  SERIAL_OUTPUT_PORT &= ~SERIAL_OUTPUT_PIN;
}

inline void writeZero(void)
{
  SERIAL_OUTPUT_PORT |= SERIAL_OUTPUT_PIN;
  _no_operation();

  SERIAL_OUTPUT_PORT &= ~SERIAL_OUTPUT_PIN;

}


// Set pixel color from 'packed' 32-bit RGB color:
void setPixelColor(struct WS2812B_Strip *strip, uint16_t pixelIndex, uint32_t color)
{
  if(pixelIndex < strip->numberOfPixels)
  {
    uint16_t r = (uint8_t)(color >> 16);
    uint16_t g = (uint8_t)(color >>  8);
    uint16_t b = (uint8_t)(color);

    if(strip->brightness)
    { // See notes in setBrightness()
      r = (r * strip->brightness) >> 8;
      g = (g * strip->brightness) >> 8;
      b = (b * strip->brightness) >> 8;
    }
    uint8_t *p = &strip->pixels[pixelIndex * 3];
    p[1] = r;
    p[0] = g;
    p[2] = b;
  }
}



// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void setBrightness(struct WS2812B_Strip *strip, uint8_t brightness)
{
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = brightness + 1;
  if(newBrightness != strip->brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = strip->pixels,
             oldBrightness = strip->brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(strip->brightness == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    uint16_t i;
    for(i = 0; i < strip->numberOfBytes; i++)
    {
      c = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    strip->brightness = newBrightness;
  }
}

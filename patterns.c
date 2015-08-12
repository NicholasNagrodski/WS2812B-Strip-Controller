/*
 * patterns.c
 *
 *  Created on: May 2, 2015
 *      Author: TWeaK
 */


#include "patterns.h"

//******************************************************************************
//******************************************************************************
//  PATTERNS
//******************************************************************************
//******************************************************************************


// Cycles through each LED pixel, testing each color along the way.
void testColor(struct WS2812B_Strip *strip, const uint8_t red, const uint8_t green, const uint8_t blue, const uint32_t delayTime)
{
  if (strip->inInterrupt == true)
  {
    return;
  }

  colorWipe(strip, color(red, green, blue), delayTime);
  return;
}


// Fill the dots one after the other with a color, like wiping a paint bursh
//  delayTime is the number of ms between each pixel being lit.
void colorWipe(struct WS2812B_Strip *strip, const uint32_t color, const uint32_t delayTime)
{
  uint16_t pixelIndex;

  for(pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex++)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

  	setPixelColor(strip, pixelIndex, color);
    show(strip);
    delay_ms(delayTime/NUMBER_OF_PIXELS);
  }
  return;
}

void fillStripWithSolidColor(struct WS2812B_Strip *strip, const uint32_t color)
{
  uint16_t pixelIndex;

  for(pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex++)
  {
	setPixelColor(strip, pixelIndex, color);
  }
  show(strip);
  return;
}

void policeLights(struct WS2812B_Strip * strip, const uint32_t delayTime)
{
  // Flash every other pixel: RED,BLUE,RED,BLUE ... BLUE,RED,BLUE,RED ...
  uint16_t i;

  for (i = 0; i < strip->numberOfPixels; i += 2)
  {
    setPixelColor(strip, i, RED);
  }
  for (i = 1; i < strip->numberOfPixels; i += 2)
  {
    setPixelColor(strip, i, BLUE);
  }
  show(strip);
  delay_ms(delayTime);

  for (i = 0; i < strip->numberOfPixels; i += 2)
  {
    setPixelColor(strip, i, BLUE);
  }
  for (i = 1; i < strip->numberOfPixels; i +=2)
  {
    setPixelColor(strip, i, RED);
  }
  show(strip);
  delay_ms(delayTime);

  strip->inInterrupt = false;
  return;
}

void rainbow(struct WS2812B_Strip *strip, const uint8_t delayTime)
{
  uint16_t pixelIndex, j;

  clear(strip);
  show(strip);

  for(j = 0; j < 256; j++)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

	for(pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex++)
    {
      setPixelColor(strip, pixelIndex, Wheel((pixelIndex+j) & 255));
    }

    show(strip);
    delay_ms(delayTime);
  }

  strip->inInterrupt = false;
  return;
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(struct WS2812B_Strip *strip, const uint8_t delayTime)
{
  uint16_t j;
  uint16_t pixelIndex;

  clear(strip);
  show(strip);

  // cycles of all colors on wheel
  for(j = 0; j < 256; j++)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

	for(pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex++)
    {
      setPixelColor(strip, pixelIndex, Wheel(((pixelIndex * 256 / strip->numberOfPixels) + j) & 255));
    }

	show(strip);
    delay_ms(delayTime);
  }

  strip->inInterrupt = false;
  return;
}


//Theatre-style crawling lights.
void theaterChase(struct WS2812B_Strip *strip, uint32_t c, const uint8_t delayTime)
{
  uint16_t pixelIndex, j;

  clear(strip);
  show(strip);

  for (pixelIndex = 0; pixelIndex < 3; pixelIndex++)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

    // Turn every third pixel on
    for (j = 0; j < strip->numberOfPixels; j += 3)
    {
      setPixelColor(strip, pixelIndex + j, c);
    }

    show(strip);
    delay_ms(delayTime);

    // Turn every third pixel off
    for (j = 0; j < strip->numberOfPixels; j += 3)
    {
      setPixelColor(strip, pixelIndex + j, 0);
    }
  }

  strip->inInterrupt = false;
  return;
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(struct WS2812B_Strip *strip, const uint8_t delayTime)
{
  // cycle all 256 colors in the wheel
  uint16_t pixelIndex;
  uint16_t j, q;

  clear(strip);
  show(strip);

  for (j = 0; j < 256; j += 4)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

    for (q = 0; q < 3; q++)
    {
      //turn every third pixel on
      for (pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex += 3)
      {
    	setPixelColor(strip, pixelIndex+q, Wheel( (pixelIndex+j) % 255));
      }
      show(strip);
      delay_ms(delayTime);

      //turn every third pixel off
      for (pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex += 3)
      {
    	setPixelColor(strip, pixelIndex+q, 0);
      }
    }
  }

  strip->inInterrupt = false;
  return;
}


// Breath a color, for a total breathe time of...
void breathe(struct WS2812B_Strip *strip, uint32_t color, const uint32_t breathCycleTime)
{
  uint8_t oldBrightness = strip->brightness;
  uint16_t pixelIndex;
  uint8_t j;

  clear(strip);
  show(strip);

  for (pixelIndex = 0; pixelIndex < strip->numberOfPixels; pixelIndex++)
  {
    setPixelColor(strip, pixelIndex, color);
  }

  // Breathe in.
  for(j = 1; j < 255; j++)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

	setBrightness(strip, j);
    show(strip);
	delay_ms(breathCycleTime/512);
  }

  // pause in.
  delay_ms(breathCycleTime/16);

  // Breathe out.
  for(j = 255; j > 0; j--)
  {
	if (strip->breakFromPattern == true)
	{
	  return;
	}

    setBrightness(strip, j);
    show(strip);
	delay_ms(breathCycleTime/512);
  }

  // pause out.
  delay_ms(breathCycleTime/16);

  strip->brightness = oldBrightness;
  return;
}

//******************************************************************************
//******************************************************************************
//   PATTERN HELPER FUNCTIONS
//******************************************************************************
//******************************************************************************

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t wheelPosition) {
  wheelPosition = 255 - wheelPosition;
  if(wheelPosition < 85)
  {
    return color(255 - wheelPosition * 3, 0, wheelPosition * 3);
  }
  else if(wheelPosition < 170)
  {
    wheelPosition -= 85;
    return color(0, wheelPosition * 3, 255 - wheelPosition * 3);
  }
  else
  {
    wheelPosition -= 170;
    return color(wheelPosition * 3, 255 - wheelPosition * 3, 0);
  }
}

/*
// Exits pattern functions if there is an ISR.
inline void returnIfInISR(struct WS2812B_Strip *strip)
{
  if (strip->inInterrupt == true)
  {
    strip->inInterrupt = false;
    return;
  }
}
*/

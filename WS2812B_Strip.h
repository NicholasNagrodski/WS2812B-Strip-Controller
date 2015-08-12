/*
 * WS2812B_Strip.h
 *
 *  Created on: Apr 1, 2015
 *      Author: TWeaK
 */

#ifndef WS2812B_STRIP_H_
#define WS2812B_STRIP_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

struct WS2812B_Strip {
    uint16_t numberOfPixels;   // Number of RGB LEDs in strip
    uint16_t numberOfBytes;    // Size of 'pixels' buffer below

    uint8_t brightness;    // Brightness level (0-255, off - fully on)
    uint8_t pixels[3 * NUMBER_OF_PIXELS];       // Holds LED color values (3 bytes each)

    bool inShow;
    bool inInterrupt;
    bool breakFromPattern;
};

void create(struct WS2812B_Strip *strip, const uint16_t numberOfPixels);

void show(struct WS2812B_Strip *strip);
void inline writeOne(void);
void inline writeZero(void);

void clear(struct WS2812B_Strip *strip);
void setPixelColor(struct WS2812B_Strip *strip, uint16_t pixelIndex, uint32_t color);
void setBrightness(struct WS2812B_Strip *strip, uint8_t brightness);

#endif // WS2812B_STRIP_H_

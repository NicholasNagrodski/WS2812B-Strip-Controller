/*
 * patterns.h
 *
 *  Created on: May 2, 2015
 *      Author: TWeaK
 */

#ifndef PATTERNS_H_
#define PATTERNS_H_

#include <stdint.h>
#include <stdbool.h>
#include "WS2812B_Strip.h"

// Callable Patterns.
void testColor(struct WS2812B_Strip *strip, const uint8_t red, const uint8_t green,const uint8_t blue, const uint32_t delayTime);
void colorWipe(struct WS2812B_Strip *strip, uint32_t color, const uint32_t delayTime);
void fillStripWithSolidColor(struct WS2812B_Strip *strip, const uint32_t color);
void policeLights(struct WS2812B_Strip *strip, const uint32_t delayTime);
void rainbow(struct WS2812B_Strip *strip, const uint8_t delayTime);
void rainbowCycle(struct WS2812B_Strip *strip, const uint8_t delayTime);
void theaterChase(struct WS2812B_Strip *strip, uint32_t color, const uint8_t delayTime);
void theaterChaseRainbow(struct WS2812B_Strip *strip, const uint8_t delayTime);
void breathe(struct WS2812B_Strip *strip, uint32_t color, const uint32_t delayTime);

// Pattern helper functions
uint32_t color(uint8_t r, uint8_t g, uint8_t b);
uint32_t Wheel(uint8_t WheelPososition);
inline void returnIfInISR(struct WS2812B_Strip *strip);

#endif /* PATTERNS_H_ */

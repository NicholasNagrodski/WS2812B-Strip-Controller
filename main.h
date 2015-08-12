

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>

#define RED   0xFF0000
#define GREEN 0x00FF00
#define BLUE  0x0000FF

// Define our output port and pin for:
// 1. Our STATUS_LED
// 2. Our SERIAL_DATA
// 3. Our PATTERN_STATE_CHANGE_SWITCH
#define STATUS_LED_PORT_DIR       P3DIR
#define STATUS_LED_PORT           P3OUT
#define STATUS_LED_PIN            BIT5
#define STATUS_LED_PIN_LOW        BIT4   // Logic low pin for our status LED.

#define SERIAL_OUTPUT_PORT_DIR    P1DIR
#define SERIAL_OUTPUT_PORT        P1OUT
#define SERIAL_OUTPUT_PIN         BIT7

#define PSC_SW_PORT_OUT P1OUT  // Choose our interrupt PORT.
#define PSC_SW_PORT_IN  P1IN   //
#define PSC_SW_PIN      BIT6   // Choose our pattern switch button.
#define PSC_SW_PIN_LOW  BIT5   // Logic low pin for our PSC switch.
#define PSC_SW_PORT_REN P1REN  // PATTERN STATE CHANGE SWITCH REN - Resistor ENable. internal pull-up resistors for PORTx
#define PSC_SW_PORT_IE  P1IE   // PATTERN STATE CHANGE SWITCH IE  - Interrupt Enable for PORTx
#define PSC_SW_PORT_IES P1IES  // PATTERN STATE CHANGE SWITCH IES - Interrupt Edge Select for PORTx (We want Hi/Lo trigger)
#define PSC_SW_PORT_IFG P1IFG  // PATTERN STATE CHANGE SWITCH IFG - Interrupt Flag for PORTx


// Number of pixels cannot excede the available RAM on your device.  It takes 3 bytes of RAM per pixel
// This code uses XXX RAM to run the software and you NEED to account for that
// I.E., 1kiB RAM means you cannot have more than ~430 pixels on your strip.
#define NUMBER_OF_PIXELS 38  // 38 Pixels on the Hat!!!

enum pattern {
	patternRGB,
	patternColorWipe,
	patternPoliceLights,
	patternRainbow,
	patternRainbowCycle,
	patternTheaterChase,
	patternTheaterChaseRainbow,
	patternBreathe,
	NUMBER_OF_PATTERNS
};

inline void ISR_Sleep();

void blinkStatusLED(const uint32_t blinkRate, const uint8_t blinkCount);
void delay_ms(uint32_t delayTime);

#endif // MAIN_H_

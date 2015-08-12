/*
 * AUTHOR: Nicholas Nagrodski
 * DATE:   2015/03/31
 * FOR:    GETFPV.com
 *
 * DESCRIPTION:
 *   SW driver for a WS2812B (serial RGB LED).
 *   Changes output patterns based on a button press.
 *
 * 24-bit color info:
 *   [ G7 G6 ... G1 G0 R7 R6 ... R1 R0 B7 B6 ... B1 B0 ]
 *   30uS per 24-bits
 *
 *                MSP430F2272
 *             -----------------
 *         /|\|              XIN|-
 *          | |                 |
 *          --|RST          XOUT|-
 *            |                 |
 *            |             P1.0|--> switch to change pattern UP   (active LOW)
 *            |             p1.1|--> switch to change pattern DOWN (active LOW)
 *            |             P4.4|--> Status LED.
 *            |             P4.5|--> SERIAL DATA for RGB LED control
 *            |                 |
 *                     .
 *                     .
 *                     .
 *
 ******************************************************************************
 *                                 INDEX                                      *
 ******************************************************************************
 *
 * 1. INITIALIZATIONS
 * 1.1 CLOCK SETUP
 * 1.2 PORT RESETS (For low power consumption)
 * 1.3 Interrupt setup for "PATTERN STATE CHANGE SWITCH" PORTx, PINx.
 *
 * 2. Main Loop.
 * 3. Interrupt routine for pattern change switch.
 * 4. Patterns.
 */

#include <msp430f2272.h>
#include "main.h"
#include "patterns.h"
#include "WS2812B_Strip.h"

struct WS2812B_Strip strip;
enum pattern patternState = NUMBER_OF_PATTERNS;

// 1.0 - Initialzations
// 1.1 - CLOCK SETUP
// 1.2 - PORT RESETS (For low power consumption)
// 1.3 - Interrupt setup for "PATTERN STATE CHANGE SWITCH" PORTx, PINx.
// 1.4 - Set outputs for the Status LED pin and the Serial TX pin.
// 1.5 - Initialize all pixels to 'off'
// 2.0 - Shutdown CPU, enable global interrupts.
// 3.0 - Main loop.
int main(void)
{
  // 1.1 - CLOCK SETUP
  WDTCTL = WDTPW | WDTHOLD;	  // Stop watchdog timer
  DCOCTL |= CALDCO_16MHZ;     // Set DCO CLK 16MHz
  BCSCTL1 |= CALBC1_16MHZ;


  // 1.2 - PORT RESETS (For low power consumption)
  // Configure as outputs, logic low, with pulldown resistors enabled.
  P1OUT &= 0x00;
  P1DIR &= 0xFF;
  P1REN &= 0xFF;
  P2DIR &= 0xFF;
  P2OUT &= 0x00;
  P2REN &= 0xFF;
  P3DIR &= 0xFF;
  P3OUT &= 0x00;
  P3REN &= 0xFF;
  P4DIR &= 0xFF;
  P4OUT &= 0x00;
  P4REN &= 0xFF;

  // 1.3 - Interrupt setup for "PATTERN STATE CHANGE SWITCH" PORTx, PINx.
  PSC_SW_PORT_REN &= 0x00;
  PSC_SW_PORT_REN |= PSC_SW_PIN;      // Enable internal pull-up resistors for PORTx.
  PSC_SW_PORT_OUT |= PSC_SW_PIN;      // Set PORTx to pull-up mode for PINx
  PSC_SW_PORT_OUT |= PSC_SW_PIN_LOW;  // Logic low pin.
  PSC_SW_PORT_IE  |= PSC_SW_PIN;      // Set PORTx interrupt enabled
  PSC_SW_PORT_IES |= PSC_SW_PIN;      // Set PORTx to trigger an interrupt on a Hi/Lo edge.
  PSC_SW_PORT_IFG &= ~PSC_SW_PIN;     // Clear PORTx Interrupt FlaG
  PSC_SW_PORT_OUT &= ~PSC_SW_PIN_LOW;

  // 1.4 Set outputs for the Status LED pin and the Serial TX pin.
  STATUS_LED_PORT_DIR    |= STATUS_LED_PIN;
  STATUS_LED_PORT_DIR    |= STATUS_LED_PIN_LOW;
  STATUS_LED_PORT        &= ~STATUS_LED_PIN;
  STATUS_LED_PORT        &= ~STATUS_LED_PIN_LOW; // Set the potential to zero

  SERIAL_OUTPUT_PORT_DIR |= SERIAL_OUTPUT_PIN;
  SERIAL_OUTPUT_PORT &= ~SERIAL_OUTPUT_PIN;

  // 1.5 - Initialize all pixels to 'off'
  create(&strip, NUMBER_OF_PIXELS);
  show(&strip);
  setBrightness(&strip, 64);  // 64/255 ~= 25% intensity.

  // 2.0 - Shutdown CPU, enable global interrupts.
  __bis_SR_register(LPM0_bits | GIE);

  // 3.0 - Main loop.
  //  Button press triggers an interrupt to change the patternState.
  //  Repeat that pattern forever until another button press.
  while (1)
  {
	while (patternState == patternRGB)
    {
      fillStripWithSolidColor(&strip, color(0xFF,    0,    0));
      delay_ms(1000);
      fillStripWithSolidColor(&strip, color(   0, 0xFF,    0));
      delay_ms(1000);
      fillStripWithSolidColor(&strip, color(   0,    0, 0xFF));
      delay_ms(1000);

      strip.breakFromPattern = false;
    }
    while (patternState == patternColorWipe)
    // R, G, B, Cyan, Yellow, Magenta, White test Case.
    {
      testColor(&strip, 255,   0,   0, 1000); // Red
      testColor(&strip, 255, 255,   0, 1000); // Yellow
      testColor(&strip,   0, 255,   0, 1000); // Green
      testColor(&strip,   0, 255, 255, 1000); // Cyan
      testColor(&strip,   0,   0, 255, 1000); // Blue
      testColor(&strip, 255,   0, 255, 1000); // Magenta
      testColor(&strip, 255, 255, 255, 1000); // White

      strip.breakFromPattern = false;
    }
    while (patternState == patternPoliceLights)
    {
      policeLights(&strip, 150);
      strip.breakFromPattern = false;
    }
    while (patternState == patternRainbow)
    {
      rainbow(&strip, 15);
      strip.breakFromPattern = false;
    }
    while (patternState == patternRainbowCycle)
    {
      rainbowCycle(&strip, 5);
      strip.breakFromPattern = false;
    }
    while (patternState == patternTheaterChase)
    {
      theaterChase(&strip, BLUE, 100);
      strip.breakFromPattern = false;
    }
    while (patternState == patternTheaterChaseRainbow)
    {
      theaterChaseRainbow(&strip, 100);
      strip.breakFromPattern = false;
    }
    while (patternState == patternBreathe)
    {
      breathe(&strip, BLUE, 2000);
      breathe(&strip, RED, 2000);
      breathe(&strip, GREEN, 2000);

      strip.breakFromPattern = false;
    }
    if (patternState == NUMBER_OF_PATTERNS)
      patternState = patternRGB;

  } // END MAIN LOOP
  //return 0;
}



//******************************************************************************
//******************************************************************************
//   3.0 INTERRUPT SERVICE ROUTINE
//******************************************************************************
//******************************************************************************

// Port 1 interrupt service routine
// Used to:
//  A. Change patterns.              (buttonHold < 1000ms)
//  B. Change Brightness.            (buttonHold > 1000ms and buttonHold < 2000ms)
//  C. Put the system in sleep mode. (buttonHold > 3000ms)
//
//  Psuedo:
// 1.0 - Blank the output LED Strip.
// 2.0 - Start our button hold count
// 2.1 - Pattern change if button hold < 1000ms.
//  2.1.1 - Flash for status change.
//  2.1.2 - Change the pattern
// 2.2 - Change Brightness.
//  2.1.1 - Flash for status change.
//  2.2.2 - Change the brightness.
// 3.0 - Clear our PORT interrupt flag.
// 4.0 - Turn CPU on.
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  __delay_cycles(800000);   // 50ms Debounce.
  if (PSC_SW_PORT_IFG & PSC_SW_PIN)
  {
	if (strip.inInterrupt == true)
	{
	  PSC_SW_PORT_IFG &=  ~PSC_SW_PIN;
	  return;
	}
	// Clear our PORT interrupt flag.
    PSC_SW_PORT_IFG &=  ~PSC_SW_PIN;

    // 1.0 Blank the output LED Strip.
    strip.inInterrupt = true;
    strip.breakFromPattern  = true;
	strip.inShow = false;
    clear(&strip);
    show(&strip);

	// 2.0 Start our button hold count
	volatile uint32_t buttonHold_ms = 0;
    while ((PSC_SW_PORT_IN & PSC_SW_PIN) == false)
    {
      __delay_cycles(16000);  // 1ms
      buttonHold_ms++;

      if (buttonHold_ms > 3000)
      {
        volatile uint8_t i;
    	for (i = 6; i != 0; i--)
    	{
   		  STATUS_LED_PORT ^= STATUS_LED_PIN;
   		  __delay_cycles(8000000); // 500ms.
   		}

    	strip.inInterrupt = false;
   		PSC_SW_PORT_IFG &=  ~PSC_SW_PIN;
   		__bis_SR_register(LPM0_bits | GIE);  // Turn off the CPU, enable global interrupts.
    	return;
      }
    }

    // 4.1 - Pattern change if button hold < 1000ms.
    if (buttonHold_ms < 1000)
    {
      // 4.1.1 - Flash for status change.
      volatile uint8_t i;  // i NEEDS to be an even number!
      for (i = 10; i != 0; i--)
      {
        STATUS_LED_PORT ^= STATUS_LED_PIN;
        __delay_cycles(1600000); // 100ms.
      }

      // 4.1.2 - Change the pattern
      if (patternState == NUMBER_OF_PATTERNS)
        patternState = patternRGB;
  	  else
  	    patternState++;
    }
    // 4.2 Change Brightness.
    else
    //else if ((buttonHold > 1000) && (buttonHold_ms < 3000))  // 1000ms < buttonHold_ms < 3000ms
    {
      // 2.1.1 - Flash for status change.
      uint8_t i;  // i NEEDS to be an even number!
      for (i = 20; i != 0; i--)
      {
        STATUS_LED_PORT ^= STATUS_LED_PIN;
        __delay_cycles(400000); // 50ms.
      }

      // 4.2.2 - Change the brightness.
      if (strip.brightness != 255)
    	strip.brightness = 255; // 255/255  = 100.0% intensity.
      else
    	strip.brightness = 64;  // 64/255  = 25.0% intensity.
    }
  }

  // 5.0 Wake CPU.
  __bic_SR_register_on_exit(LPM0_bits);
  strip.inInterrupt = false;
  PSC_SW_PORT_IFG &=  ~PSC_SW_PIN;
  return;
}



void blinkStatusLED(const uint32_t blinkDelay, const uint8_t blinkCount)
{
  uint8_t i = 2 * blinkCount; // Make sure we have an even on/off ratio.
  while (i != 0)
  {
    STATUS_LED_PORT ^= STATUS_LED_PIN;
    delay_ms(blinkDelay);
    i--;
  }
}

// TimerA ISR. Used for delay
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  // wake on CCR0 count match
  TA0CCTL0 = 0;
  //__enable_interrupt();
  //__bic_SR_register_on_exit(LPM0_bits);
}

void delay_ms(uint32_t delayTime)
{


  // change according to your SMCLK frequency
  TA0CCR0 = 16000;
  TA0CTL = TASSEL_2 + MC_2;  // Use SMCLK + continous up
  while ((delayTime != 0) && (strip.inInterrupt == false))
  {
    TA0R = 0;                // Counter Register.
    TA0CCTL0 = CCIE;         // Enable Capture/control.
    while(TA0R < 16000)
    {
      //__bis_SR_register(LPM0_bits | GIE);
      __no_operation();
    }
    delayTime--;
  }
  TA0CTL = 0;
}

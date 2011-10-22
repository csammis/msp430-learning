/*
 * debounce.h
 * Common code for debouncing an input signal
 *
 * USAGE:
 *  Calling code should #define the method of debounce before #include-ing this file.
 *      #define DEBOUNCE_USING_WDT if the watchdog timer is available for use.
 *      Otherwise #define DEBOUNCE_USING_DELAY to delay execution for a number of cycles.
 *      One of these must be specified or the compiler will throw an error.
 *
 * EXAMPLE:
 *  #include <io.h>
 *  #include <signal.h>
 *  
 *  #define DEBOUNCE_USING_WDT
 *  #include "debounce.h"
 *
 *  ...
 *
 *  input_isr()
 *  {
 *     // Debounce P1.3
 *     debounce(0x01, BIT3);
 *  }
 *
 * Copyright (c) 2011, Chris Sammis (http://csammisrun.net)
 * Source released under the MIT License (http://www.opensource.org/licenses/mit-license.php)
 *
 */

#ifndef DEBOUNCE_USING_WDT
#ifndef DEBOUNCE_USING_DELAY
#error "One of DEBOUNCE_USING_WDT or DEBOUNCE_USING_DELAY must be #defined."
#endif
#endif

// Declare variables for storing port and pin information.
// These are unnecessary if using a delay so we won't waste space.
#ifdef DEBOUNCE_USING_WDT
unsigned char ucWdt_Debounce_Port = 0x00;
unsigned char ucWdt_Debounce_Pin = 0x00;
#endif

// General debounce routine
//
// Debouncing disables the input interrupt briefly so that the ISR
// can continue without being re-entered unexpectedly.
void debounce(unsigned char ucPort, unsigned char ucPin)
{
    // Disable interrupts on the specified pin and clear the pending interrupt flag
    if (ucPort == 0x01)
    {
        P1IE  &= ~ucPin;
        P1IFG &= ~ucPin;
    }
    else if (ucPort == 0x02)
    {
        P2IE  &= ~ucPin;
        P2IFG &= ~ucPin;
    }
    else return;

#ifdef DEBOUNCE_USING_WDT
    // Store the port and pin so that it can be accessed in the WDT ISR
    ucWdt_Debounce_Port = ucPort;
    ucWdt_Debounce_Pin = ucPin;

    // Configure the WDT to fire off after SMCLK (1MHz) / 512 = 29ms
    WDTCTL = (WDTPW + WDTTMSEL + WDTCNTCL + WDTIS1);

    IFG1 &= ~WDTIFG;    // Clear the WDT interrupt flag
    IE1 |= WDTIE;       // Enable WDT interrupts
#else

    __asm__
    (
        "mov #1000, r2  \n\t"
        "dec r2         \n\t"
        "jnz $-2        \n\t"
    );

    // Clear the pending interrupt flags and re-enable the interrupt
    if (ucPort == 0x01)
    {
        P1IFG &= ~ucPin;
        P1IE  |=  ucPin;
    }
    else if (ucPort == 0x02)
    {
        P2IFG &= ~ucPin;
        P2IE  |=  ucPin;
    }
#endif
}

#ifdef DEBOUNCE_USING_WDT
// WDT interrupt vector: re-enables interrupts on ucWdt_Debounce_Port.ucWdt_Debounce_Pin
interrupt(WDT_VECTOR) wdt_isr()
{
    IE1 &= ~WDTIE;   // Disable the WDT interrupt
    IFG1 &= ~WDTIFG; // Clear the WDT interrupt flag

    WDTCTL = WDTPW + WDTHOLD; // Stop the WDT again
    
    if (ucWdt_Debounce_Port == 0x01)
    {
        P1IFG &= ~ucWdt_Debounce_Pin;   // Clear the flag for the debounced pin
        P1IE |= ucWdt_Debounce_Pin;     // Re-enable the interrupts on the debounced pin
    }
    else if (ucWdt_Debounce_Port == 0x02)
    {
        P2IFG &= ~ucWdt_Debounce_Pin;
        P1IE  |=  ucWdt_Debounce_Pin;
    }
}
#endif

//eof



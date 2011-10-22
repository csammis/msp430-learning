/*
 * Babby's First MSP430G2xx Program
 * Toggle an LED on the Launchpad on button press
 *
 * Copyright (c) 2011, Chris Sammis (http://csammisrun.net)
 * Source released under the MIT License (http://www.opensource.org/licenses/mit-license.php)
 *
 */

#include <io.h>
#include <signal.h>

#define DEBOUNCE_USING_DELAY

#include "debounce.h"

int main()
{
    // Stop the watchdog timer
    WDTCTL = WDTPW + WDTHOLD;   // WDTPW is a Magic Number (MSP430G2XX user guide section 10.3)

    // Initialize the clocks
    BCSCTL1  = CALBC1_1MHZ;
    DCOCTL   = CALDCO_1MHZ;
    BCSCTL2 &= ~DIVS_3;

    // Configure output to the Launchpad green LED
    P1DIR  = BIT6;  // Set P1.6 (green LED on Launchpad) to output
    P1OUT |= BIT6;  // Set P1.6 high to turn on the LED
    
    // Configure input from the Launchpad switch (P1.3)
    P1IE   = BIT3;  // Enable interrupts from P1.3
    P1OUT |= BIT3;  // Set P1.3 output to high...
    P1REN  = BIT3;  // ...and then enable the pull resistor (P1OUT.3 high specifies "up")

    __bis_SR_register(CPUOFF | GIE); // Switch to LPM0 and enable interrupts

    return 0;
}

// General ISR to switch interrupts from P1
//
// P1IFG has bits set indicating which pin caused the interrupt
interrupt(PORT1_VECTOR) p1_isr()
{ 
    if (P1IFG & BIT3)
    {
        debounce(0x01, BIT3);
        P1OUT ^= BIT6;  // Toggle P1.6's state
    }
}
//eof



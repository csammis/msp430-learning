/*
 * pwm1
 * First shot at writing some PWM code
 *
 * Copyright (c) 2011, Chris Sammis (http://csammisrun.net)
 * Source released under the MIT License (http://www.opensource.org/licenses/mit-license.php)
 *
 */

#include <io.h>
#include <signal.h>

#define DEBOUNCE_USING_WDT

#include "debounce.h"

int main()
{
    // Stop the watchdog timer
    WDTCTL = WDTPW + WDTHOLD;

    // Configure timers - DC0 to 1MHz and SMCLK to 1/8th that (125KHz)
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 = DIVS_3;

    // Tie P1.6 (green LED) to the output of TimerA0.1
    P1DIR |= BIT6;
    P1SEL |= BIT6;

    // Configure input from the Launchpad switch (P1.3)
    P1IE   = BIT3;
    P1OUT |= BIT3;
    P1REN  = BIT3;

    TACCR0 = 625; // SMCLK / 625 sets the PWM frequency to 200Hz. I guess this is good.

    TACTL = TASSEL_2 | MC_1;    // Source TimerA from SMCLK and count up from 0 to TACCR0
    TACCTL1 = OUTMOD_7; // When TimerA0.1 hits 0, set output high; when it hits TACCR1 set output low.
                        // The higher TACCR1 the longer the output stays high.
    
    TACCR1 = 0; // Initial brightness:  zero

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

        if (TACCR1 == 200)
        {
            TACCR1 = 0;
        }
        else
        {
            TACCR1 += 50;
        }
    }
}
//eof



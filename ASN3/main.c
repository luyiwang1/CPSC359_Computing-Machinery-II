// Minh Hang Chu 30074056
// CPSC 359 - Fall 2019 - University of Calgary
// Assignment 3

// This assignment is completed based on sources from Tutorial notes and Lecture notes which are uploaded on D2L 

// This program sets up GPIO pin 23 and pin 22 as an input pin, and sets it to generate
// an interrupt whenever a rising edge is detected. The pin is assumed to
// be connected to a push button switch on a breadboard. When the button is
// pushed, a 3.3V level will be applied to the pin.
// If the pin 23 is pressed, sequence light turned on is light 1 (blue), light2 (yellow) and light 3 (red).
// Each light will be on for 0.5 s and off for 0.5s
// If the pin 22 is pressed, sequence light turned on is light 3 (red), light2 (yellow) and light 1 (blue).
// Each light will be on for 0.25s and off for 0.5s

// Include files
#include "uart.h"
#include "sysreg.h"
#include "gpio.h"
#include "irq.h"
#include "systimer.h"


// Function prototypes and declare global shareValue
void init_GPIO23_to_risingEdgeInterrupt();
void init_GPIO22_to_risingEdgeInterrupt();

void init_GPIO4_to_output();
void set_GPIO4();
void clear_GPIO4();

void init_GPIO17_to_output();
void set_GPIO17();
void clear_GPIO17();

void init_GPIO27_to_output();
void set_GPIO27();
void clear_GPIO27();

unsigned int sharedValue;

////////////////////////////////////////////////////////////////////////////////////////////////////////////  MAIN  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function first prints out the values of some system
//                  registers for diagnostic purposes. It then initializes
//                  GPIO pin 23 and pin 22 to be an input pin that generates an interrupt
//                  (IRQ exception) whenever a rising edge occurs on the pin.
//                  The function then goes into an infinite loop, where the
//                  shared global variable is continually checked. If the
//                  interrupt service routine changes the shared variable,
//                  then this is detected in the loop, and the current value
//                  is printed out.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    unsigned int r;
    r = 0x0000FFFF;
    // Set up the UART serial port
    uart_init();

    // Enable IRQ Exceptions
    enableIRQ(); 

    // Set up GPIO pin #23 to input and so that it triggers
    // an interrupt when a rising edge is detected
    init_GPIO23_to_risingEdgeInterrupt();
    init_GPIO22_to_risingEdgeInterrupt();

    // Set up GPIO pin
    // GPIO pin for light 1,2,3 is GPIO4, GPIO17 and GPIO27
    init_GPIO4_to_output();   // light 1
    init_GPIO17_to_output();  // light 2
    init_GPIO27_to_output();  // light 3

    // Print out a message to the console
    uart_puts("Starting program.\n");

    // Initialize the sharedValue global variable and
    // and set the local variable to be same value
    sharedValue = 1;   

    // Infinite loop, blinking the LED in sequence on and off
    while (1) {
      // If shareValue is 1, pin 23 is pressed and the program starts sequence 1
      // Sequence 1 will be light 1, 2, 3 turned on anf off for 0.5s each
      if (sharedValue == 1){
	  uart_puts("1 2 3 \n");
          // Turn on and off the LED 1 (blue) with duration 0.5s
          set_GPIO4();
          microsecond_delay(500000);
          clear_GPIO4();
          microsecond_delay(500000);

	   // Turn on and off the LED 2 (yellow) with duration 0.5s
          set_GPIO17();
          microsecond_delay(500000);
          clear_GPIO17();
          microsecond_delay(500000);

	   // Turn on and off the LED 3 (red) with duration 0.5s
          set_GPIO27();
          microsecond_delay(500000);
          clear_GPIO27();
          microsecond_delay(500000);
      }

      else if (sharedValue == 0){
	  uart_puts("3 2 1 \n");
	  // Turn on and off the LED 3 (red) with duration 0.25s
          set_GPIO27();
          microsecond_delay(250000);
          clear_GPIO27();
          microsecond_delay(250000);

	  // Turn on and off the LED 2 (yellow) with duration 0.25s
          set_GPIO17();
          microsecond_delay(250000);
          clear_GPIO17();
          microsecond_delay(250000);
	  
	  // Turn on and off the LED 1 (blue) with duration 0.25s
          set_GPIO4();
          microsecond_delay(250000);
          clear_GPIO4();
          microsecond_delay(250000);
      }
      r = 0x0000FFFF;
      while (r--) {
        asm volatile("nop");
      } 
    }   
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////  GPIO 23 and 22  /////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO23_to_risingEdgeInterrupt
//                  init_GPIO22_to_risingEdgeInterrupt
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 23 and 22 to an input pin without
//                  any internal pull-up or pull-down resistors. Note that
//                  a pull-down (or pull-up) resistor must be used externally
//                  on the bread board circuit connected to the pin. Be sure
//                  that the pin high level is 5V (definitely NOT 5V).
//                  GPIO pin 23 (or 22) is also set to trigger an interrupt on a
//                  rising edge, and GPIO interrupts are enabled on the
//                  interrupt controller.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO23_to_risingEdgeInterrupt()
{
    register unsigned int r;
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL2;

    // Clear bits 21 - 23. This is the field FSEL23, which maps to GPIO pin 23.
    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin
    r &= ~(0x7 << 9);

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 1
    *GPFSEL2 = r;

    // Disable the pull-up/pull-down control line for GPIO pin 23. We follow the
    // procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. We
    // will pull down the pin using an external resistor connected to ground.

    // Disable internal pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register 
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time 
    // for the control signal
    r = 150;
    while (r--) {
        asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 23 to
    // clock in the control signal for GPIO pin 23. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << 23);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
        asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;
    
    // Set pin 23 to so that it generates an interrupt on a rising edge.
    // We do so by setting bit 23 in the GPIO Rising Edge Detect Enable
    // Register 0 to a 1 value (p. 97 in the Broadcom manual).
    *GPREN0 |= (0x1 << 23);
    
    // Enable the GPIO IRQS for ALL the GPIO pins by setting IRQ 52
    // GPIO_int[3] in the Interrupt Enable Register 2 to a 1 value.
    // See p. 117 in the Broadcom Peripherals Manual.
    *IRQ_ENABLE_IRQS_2 = (0x1 << 20);
}



void init_GPIO22_to_risingEdgeInterrupt()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL2;

    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin
    r &= ~(0x7 << 9);

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 1
    *GPFSEL2 = r;

    // Disable internal pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register 
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time 
    // for the control signal
    r = 150;
    while (r--) {
        asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 23 to
    // clock in the control signal for GPIO pin 23. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << 23);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
        asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;
    
    // Set pin 22 to so that it generates an interrupt on a rising edge.
    // We do so by setting bit 23 in the GPIO Rising Edge Detect Enable
    // Register 0 to a 1 value (p. 97 in the Broadcom manual).
    *GPREN0 |= (0x1 << 22);
    
    // Enable the GPIO IRQS for ALL the GPIO pins by setting IRQ 52
    // GPIO_int[3] in the Interrupt Enable Register 2 to a 1 value.
    // See p. 117 in the Broadcom Peripherals Manual.
    *IRQ_ENABLE_IRQS_2 = (0x1 << 20);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////  LED  //////////////////////////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------------- LED 1 ------------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO4_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 4 to an output pin without
//                  any pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////


void init_GPIO4_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 0
    r = *GPFSEL0;

    // Clear bits 12 - 14. This is the field FSEL4, which maps to GPIO pin 4.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << (4 * 3));

    // Set the field FSEL4 to 001, which sets pin 4 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    r |= (0x1 << (4 * 3));

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 1
    *GPFSEL0 = r;

    // Disable the pull-up/pull-down control line for GPIO pin 4. We follow the
    // procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. The
    // internal pull-up and pull-down resistor isn't needed for an output pin.

    // Disable pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register 
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time 
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 4 to
    // clock in the control signal for GPIO pin 4. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << 4);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO4
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 4
//                  to a 1 (high) level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO4()
{
	  register unsigned int r;
	  
	  // Put a 1 into the SET4 field of the GPIO Pin Output Set Register 0
	  r = (0x1 << 4);
	  *GPSET0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO4
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 4
//                  to a 0 (low) level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO4()
{
	  register unsigned int r;
	  
	  // Put a 1 into the CLR4 field of the GPIO Pin Output Clear Register 0
	  r = (0x1 << 4);
	  *GPCLR0 = r;
}



//----------------------------------------------------------------------------------------------- LED 2 ------------------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO17_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 17 to an output pin without
//                  any pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO17_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 0
    r = *GPFSEL1;

    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << (7 * 3));

    // Set the field FSEL4 to 001, which sets pin 17 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    r |= (0x1 << (7 * 3));

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 0
    *GPFSEL1 = r;

    // Disable the pull-up/pull-down control line for GPIO pin 17. We follow the
    // procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. The
    // internal pull-up and pull-down resistor isn't needed for an output pin.

    // Disable pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register 
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time 
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 4 to
    // clock in the control signal for GPIO pin 4. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << 4);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO17
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 17
//                  to a 1 (high) level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO17()
{
	  register unsigned int r;
	  
	  r = (0x1 << 17);
	  *GPSET0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO17
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 17
//                  to a 0 (low) level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO17()
{
	  register unsigned int r;
     
	  r = (0x1 << 17);
	  *GPCLR0 = r;
}

//-------------------------------------------------------------------------------------------------LED 3 -------------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO27_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 27 to an output pin without
//                  any pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO27_to_output()
{
    register unsigned int r;
    
    // Get the current contents of the GPIO Function Select Register 0
    r = *GPFSEL2;

    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << (7 * 3));

    // Set the field FSEL4 to 001, which sets pin 4 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    r |= (0x1 << (7 * 3));

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 2
    *GPFSEL2 = r;

    // Disable the pull-up/pull-down control line for GPIO pin 4. We follow the
    // procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. The
    // internal pull-up and pull-down resistor isn't needed for an output pin.

    // Disable pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register 
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time 
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 4 to
    // clock in the control signal for GPIO pin 4. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << 4);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO27
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 27
//                  to a 1 (high) level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO27()
{
	  register unsigned int r;
	  
	  r = (0x1 << 27);
	  *GPSET0 = r;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO27
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 27
//                  to a 0 (low) level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO27()
{
	  register unsigned int r;
	  
	  r = (0x1 << 27);
	  *GPCLR0 = r;
}








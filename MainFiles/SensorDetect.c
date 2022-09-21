//Accept and process IR signals from the remote controller

#include <SensorDetect.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "tm4c123gh6pm.h"
#include "uart0.h"

// Pin bitbands
#define OUTPUT_SENSOR    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4))) //PORT PF4

// PortF masks
#define OUTPUT_SENSOR_MASK 16

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint32_t time[50];
uint8_t bits[32];
uint8_t array[4];
uint8_t count = 0;
uint8_t bcount = 0;
uint8_t code;
bool valid = false;
uint8_t j = 0;
uint8_t temp = 0;
uint8_t sum = 0;
uint8_t k, x, y ;

uint8_t code;


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R3;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    //GPIO Configuration
    GPIO_PORTF_DEN_R |= OUTPUT_SENSOR_MASK;  //Enable PF4
    GPIO_PORTF_DIR_R &= ~OUTPUT_SENSOR_MASK; //Set PF4 to be input.

    GPIO_PORTF_IS_R &= ~OUTPUT_SENSOR_MASK;  //Set to detect the edges.
    GPIO_PORTF_IM_R &= ~OUTPUT_SENSOR_MASK;  //Turn off key press interrupts. OR it if we need to turn the interrupts on.
    GPIO_PORTF_IBE_R &= ~OUTPUT_SENSOR_MASK; //Set to detect only one edge (not both)
    GPIO_PORTF_IEV_R &= ~OUTPUT_SENSOR_MASK; //Set to detect only falling edge
    GPIO_PORTF_ICR_R |= OUTPUT_SENSOR_MASK;  //Clear all previous interrupts.
    NVIC_EN0_R |= 1 << (INT_GPIOF-16);       //Turn on interrupt for (GPIOF)
    GPIO_PORTF_PUR_R |= OUTPUT_SENSOR_MASK;
    GPIO_PORTF_IM_R |= OUTPUT_SENSOR_MASK;


    //Configure WideTimer1 as the time base. From freq_timer.c

    WTIMER3_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER3_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER3_TAMR_R |= TIMER_TAMR_TAMR_PERIOD;        //count down, periodic timer mode....count down is set with 0 at 4 bit thus we can leave it out. Default 0
    WTIMER3_TAV_R = 0xFFFFFFFF;                      //Load initial value with max value, since it is count down.
    WTIMER3_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    WTIMER3_TAPR_R = 40;                             //load a value of 40 to allow to count at 1us rate.
}


void gpioIsr()
{

    time[count] = WTIMER3_TAV_R; //Load the time array with the current value in TAV_R.
    count++;

    if (count > 1) //else it has entered the 32 bit range. Address or Data bits
    {
        if (time[count - 2] - time[count - 1] >= 13000 && time[count - 2] - time[count - 1] <= 14000)
        {
            //Passed the initial 13.5ms phase.
            putsUart0("\n\nFound leader code\n");

        }
        else if (time[count - 2] - time[count - 1] >= 800 && time[count - 2] - time[count - 1] <= 1500)
        {
            //this is a valid for bit '0'
            bits[bcount] = 0;
            putcUart0('0'); //try to printout bits.
            bcount++;

        }
        else if (time[count - 2] - time[count - 1] >= 2000 && time[count - 2] - time[count - 1] <= 5000)
        {
            //this is a valid bit '1'
            bits[bcount] = 1;
            putcUart0('1'); //try to printout bits
            bcount++;

        }
        else
        {
            //bit was invalid restart from the beginning.
            count = 0;
            bcount = 0;
        }

    }

    //Enter this if statement if code has reached the last negative edge.
    if(count >= 34)
    {

        // Process the data and compare to lab 6 values.
        uint8_t address = 0, addressComp = 0;
        uint8_t data = 0, dataComp = 0;

        uint8_t i = 0;



        //Determining decimal value of address bits

        for(i = 0; true; i++)
        {
            address |= bits[i];
            if(i == 7)
                break;
            address <<= 1;
        }



        //Determining the decimal value of addressComp bits

        for (; true; i++)
        {
            addressComp |= bits[i];
            if(i == 15)
                break;
            addressComp <<= 1;
        }

        addressComp = ~addressComp;

        //Determining decimal value of Data bits

        for(i = 16; true; i++)
        {
            data |= bits[i];
            if (i == 23)
                break;
            data <<= 1;
        }

        //Determining decimal value of DataComp bits.

        for(i = 24; true; i++)
        {
            dataComp |= bits[i];
            if (i ==31)
                break;
            dataComp <<= 1;

        }

        dataComp = ~dataComp;


        //Checking if address and data are complement of each other.
        if ((data == dataComp) && (address == addressComp))
        {
           //Set valid to true to indicate that this code is valid.
           valid = true;

        }


        if (valid == true)
        {
            putcUart0('\n');

            //begin to process code in here.
            code = data;

            if(data == 16)
            {
                putsUart0("On/Off Button");
            }
            else if (data == 208)
            {
                putsUart0("Input Button");
            }
            else if (data == 156)
            {
                putsUart0("Caption Button");
            }
            else if (data == 94)
            {
                putsUart0("Question Button");
            }
            else if (data == 30)
            {
                putsUart0("Search Button");
            }
            else if (data == 240)
            {
                putsUart0("TV Button");
            }
            else if (data == 136)
            {

                putsUart0("1 Button");
            }
            else if (data == 72)
            {
                putsUart0("2 Button");
            }
            else if (data == 200)
            {
                putsUart0("3 Button");
            }
            else if (data == 40)
            {
                putsUart0("4 Button");
            }
            else if (data == 168)
            {
                putsUart0("5 Button");
            }
            else if (data == 104)
            {
                putsUart0("6 Button");
            }
            else if (data == 232)
            {
                putsUart0("7 Button");
            }
            else if (data == 24)
            {
                putsUart0("8 Button");
            }
            else if (data == 152)
            {
                putsUart0("9 Button");
            }
            else if (data == 50)
            {
                putsUart0("List Button");
            }
            else if (data == 8)
            {

                putsUart0("0 Button");
            }
            else if (data == 88)
            {
                putsUart0("FlashBK Button");
            }
            else if (data == 64)
            {
                putsUart0("Volume+ Button");
            }
            else if (data == 192)
            {
                putsUart0("Volume- Button");
            }
            else if (data == 120)
            {
                putsUart0("Fav Button");
            }
            else if (data == 85)
            {
                putsUart0("Info Button");
            }
            else if (data == 144)
            {
                putsUart0("Mute Button");
            }
            else if (data == 0)
            {
                putsUart0("Channel+ Button");
            }
            else if (data == 128)
            {
                putsUart0("Channel- Button");
            }
            else if (data == 106)
            {
                cw(90);
                ccw(90);
                cw(90);
                ccw(90);
                putsUart0("Netflix Button");
            }
            else if (data == 62)
            {
                putsUart0("Home Button");
            }
            else if (data == 58)
            {
                putsUart0("Amazon Button");
            }
            else if (data == 194)
            {
                putsUart0("Settings Button");
            }
            else if (data == 245)
            {
                putsUart0("LiveZoom Button");
            }
            else if (data == 2)
            {

                //forward(10);
                putsUart0("Up Button");
            }
            else if (data == 96)
            {
                //cw(90);
                putsUart0("Right Button");
            }
            else if (data == 130)
            {
                putsUart0("Down Button");
            }
            else if (data == 224)
            {
                ccw(90);
                putsUart0("Left Button");
            }
            else if (data == 34)
            {

                putsUart0("OK Button");
            }
            else if (data == 20) //was at 168 in excel but actually 20
            {
                putsUart0("Back Button");
            }
            else if (data == 218)
            {
                putsUart0("Exit Button");
            }
            else if (data == 137)
            {
                putsUart0("SAP Button");
            }
            else if (data == 112)
            {
                putsUart0("Sleep Button");
            }
            else if (data == 141)
            {
                stop();
                putsUart0("Stop Button");
            }
            else if (data == 241)
            {
                putsUart0("Rewind Button");
            }
            else if (data == 13)
            {
                putsUart0("Play Button");
            }
            else if (data == 93)
            {
                putsUart0("Pause Button");
            }
            else if (data == 113)
            {
                putsUart0("FastForward Button");
            }
            else if (data == 78)
            {
                putsUart0("Red Button");
            }
            else if (data == 142)
            {
                putsUart0("Green Button");
            }
            else if (data == 198)
            {
                putsUart0("Yellow Button");
            }
            else if (data == 134)
            {
                putsUart0("Blue Button");
            }

            count = 0;
            bcount = 0;
            //valid = false;

        }

    }

    //clear the interrupt flag.
    GPIO_PORTF_ICR_R |= OUTPUT_SENSOR_MASK;
}


//Send PWM signals to motors. PWM signals vary based on param being passed in

#include <Motors.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "clock.h"



// PortF masks
#define YELLOW_LED_MASK 1 //PF0
#define RED_LED_MASK 2 //PF1
#define BLUE_LED_MASK 4 //PF2
#define GREEN_LED_MASK 8  //PF3

#define SLEEP_MASK 2 //PE1

#define LEFTS_MASK 16 //PC4
#define RIGHTS_MASK 64 //PC6


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize RGB
void initRgb()
{
    // Enable clocks

    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;
    // Enable clocks for WTIMER0
    // Enable clocks for WTIMER1
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;//Port F
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;//Port E
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;//Port C

    _delay_cycles(3);

    //Configuring the SLEEP GPIO (PE1). Pull Up High

    GPIO_PORTE_DIR_R |= SLEEP_MASK; //Enable PE1
    GPIO_PORTE_DEN_R |= SLEEP_MASK; //Enable PE1
    GPIO_PORTE_DATA_R |= SLEEP_MASK;

    //Configure Sensors Left (PC4), Right (PC6)
    GPIO_PORTC_DIR_R &= ~(LEFTS_MASK | RIGHTS_MASK); //Enable them as inputs
    GPIO_PORTC_DEN_R |= LEFTS_MASK | RIGHTS_MASK; //Enable pins PC4 and PC6
    GPIO_PORTC_PUR_R |= LEFTS_MASK | RIGHTS_MASK; //Enable pull up for both pins.
    GPIO_PORTC_AFSEL_R |= LEFTS_MASK | RIGHTS_MASK; //Set them to work with alternate function.
    GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC6_M | GPIO_PCTL_PC4_M);
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_WT0CCP0 | GPIO_PCTL_PC6_WT1CCP0;

    //Configure WideTimer1 as the time base. From freq_timer.c For PC6 Right Sensor
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge count mode, count up
    WTIMER1_CTL_R = 0;                               //
    WTIMER1_IMR_R = 0;                               // turn-off interrupts
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    NVIC_EN3_R &= ~(1 << (INT_WTIMER1A-16-96));      // turn-off interrupt 112 (WTIMER1A)
    //Configure WideTimer0 as the time base. From freq_timer.c For PC4 Left Sensor

    WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER0_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER0_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge count mode, count up
    WTIMER0_CTL_R = 0;                               //
    WTIMER0_IMR_R = 0;                               // turn-off interrupts
    WTIMER0_TAV_R = 0;                               // zero counter for first period
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    NVIC_EN3_R &= ~(1 << (INT_WTIMER0A-16-96));      // turn-off interrupt 112 (WTIMER1A)

    //Configure GPIOs
    GPIO_PORTF_DIR_R |= RED_LED_MASK | GREEN_LED_MASK | BLUE_LED_MASK | YELLOW_LED_MASK;
    GPIO_PORTF_DEN_R |= RED_LED_MASK | GREEN_LED_MASK | BLUE_LED_MASK| YELLOW_LED_MASK;
    GPIO_PORTF_AFSEL_R |= RED_LED_MASK | GREEN_LED_MASK | BLUE_LED_MASK| YELLOW_LED_MASK;
    GPIO_PORTF_PCTL_R &= ~(GPIO_PCTL_PF1_M | GPIO_PCTL_PF2_M | GPIO_PCTL_PF3_M | GPIO_PCTL_PF0_M);
    GPIO_PORTF_PCTL_R |= GPIO_PCTL_PF0_M1PWM4| GPIO_PCTL_PF1_M1PWM5 | GPIO_PCTL_PF2_M1PWM6 | GPIO_PCTL_PF3_M1PWM7;

    // Configure PWM module 0 to drive RGB back light
    // RED   on M1PWM4 (PF0), M1PWM2a
    // BLUE  on M1PWM5 (PF1), M1PWM2b
    // GREEN on M1PWM6 (PF2), M1PWM3a
    // YELLOW on M1PWM7 (PF3), M1PWM3b

    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;                // reset PWM1 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
    PWM1_2_CTL_R = 0;                                // turn-off PWM1 generator 2 (drives outs 4 and 5)
    PWM1_3_CTL_R = 0;                                // turn-off PWM1 generator 3 (drives outs 6 and 7)

    PWM1_2_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
                                                     // output 4 on PWM1, gen 2a, cmpb
    PWM1_2_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                     // output 5 on PWM1, gen 2b, cmpb
    PWM1_3_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
                                                     // output 6 on PWM1, gen 3a, cmpa
    PWM1_3_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                     // output 7 on PWM1, gen 3b, cmpb

    PWM1_2_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
    PWM1_3_LOAD_R = 1024;


    //Right Motor
    PWM1_2_CMPA_R = 0;       //Reverse
    PWM1_3_CMPB_R = 0;          //Forward


    //Left Motors
    PWM1_2_CMPB_R = 0;       //Reverse
    PWM1_3_CMPA_R = 0;    // Forward


    PWM1_2_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 2
    PWM1_3_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 3


    PWM1_ENABLE_R = PWM_ENABLE_PWM4EN |PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN; // enable outputs
}

//working
void forward(uint16_t dist_cm)
{
    uint8_t num_rot;

    WTIMER1_TAV_R = 0; // Right Sensor reset
    WTIMER0_TAV_R = 0; // Left Sensor reset

   num_rot = (dist_cm * 46)/20.5;
  // num_rot = temp * 46;

   while((WTIMER1_TAV_R < num_rot) && (WTIMER0_TAV_R < num_rot))
   {
       //Left
       PWM1_3_CMPB_R = 1023; //Forward
       PWM1_2_CMPA_R = 0;   //Reverse

       //Right
       PWM1_3_CMPA_R = 1023; //Forward
       PWM1_2_CMPB_R = 0; //Reverse


   }

   //Right Motor
   PWM1_2_CMPA_R = 0;       //Reverse
   PWM1_3_CMPB_R = 0;          //Forward


   //Left Motors
   PWM1_2_CMPB_R = 0;       //Reverse
   PWM1_3_CMPA_R = 0;          // Forward


}

//working
void reverse(uint16_t dist_cm)
{
      uint8_t num_rot;

      WTIMER1_TAV_R = 0; // Right Sensor reset
      WTIMER0_TAV_R = 0; // Left Sensor reset

      num_rot = (dist_cm * 46)/20.5;

      while((WTIMER1_TAV_R < num_rot) && (WTIMER0_TAV_R < num_rot))
         {
             //Right Motor
             PWM1_2_CMPA_R = 1023;          //Reverse
             PWM1_3_CMPB_R = 0;       //Forward


             //Left Motors
             PWM1_2_CMPB_R = 1023;       //Reverse
             PWM1_3_CMPA_R = 0;    // Forward

         }

         //Right Motor
         PWM1_2_CMPA_R = 0;       //Reverse
         PWM1_3_CMPB_R = 0;          //Forward


         //Left Motors
         PWM1_2_CMPB_R = 0;       //Reverse
         PWM1_3_CMPA_R = 0;          // Forward

}

//working
void cw(uint16_t degrees)
{
    uint8_t temp, num_rot;
    temp = (degrees*40)/360; //convert to cm. May have to be float.
    num_rot = (temp*46)/20.5; //final number of rotations. May have to be float.

    WTIMER1_TAV_R = 0; // Right Sensor reset
    WTIMER0_TAV_R = 0; // Left Sensor reset

    while (WTIMER1_TAV_R < num_rot)
    {
        //Right
        PWM1_3_CMPA_R = 1023; //Forward
        PWM1_2_CMPB_R = 0; //Reverse

    }

    //Turn off all motors.

    //Right Motor
    PWM1_2_CMPA_R = 0;          //Reverse
    PWM1_3_CMPB_R = 0;       //Forward


    //Left Motors
    PWM1_2_CMPB_R = 0;       //Reverse
    PWM1_3_CMPA_R = 0;    // Forward


}

//working
void ccw(uint16_t degrees)
{
     uint8_t temp, num_rot;
     temp = (degrees*40)/360; //convert to cm. May have to be float.
     num_rot = (temp*46)/20.5; //final number of rotations. May have to be float.

     WTIMER1_TAV_R = 0; // Right Sensor reset
     WTIMER0_TAV_R = 0; // Left Sensor reset


       while(WTIMER0_TAV_R < num_rot)
       {

           //Left
           PWM1_3_CMPB_R = 1023; //Forward
           PWM1_2_CMPA_R = 0;   //Reverse

       }

       //Turn off all motors.

       //Right Motor
       PWM1_2_CMPA_R = 0;          //Reverse
       PWM1_3_CMPB_R = 0;          //Forward


       //Left Motors
       PWM1_2_CMPB_R = 0;       //Reverse
       PWM1_3_CMPA_R = 0;       //Forward

}

void stop()
{

    PWM1_2_CMPA_R = 0;
    PWM1_2_CMPB_R = 0;
    PWM1_3_CMPA_R = 0;
    PWM1_3_CMPB_R = 0;
}

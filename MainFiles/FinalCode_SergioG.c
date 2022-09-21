



#include <Parsing.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "clock.h"
#include "SensorDetect.h"
#include "Motors.h"



   void initAllHw()
   {

       initUart0();
       initRgb();
       initHw();

   }


int main(void)
{
    initAllHw();

    USER_DATA data;
    int32_t num;
    uint8_t distance = 0;

    putsUart0("initialized\n");

	while(1)
	{
	    //USING IR SENSOR

	    //putsUart0("not enter\n");
	    if(valid)
	    {
	        putsUart0("valid is true");
	        valid = false;


            if (code == 8) //code equal 0
            {
                distance  = distance * 10 + 0;

            }

	        if (code == 136) //code equal 1
	        {

	            distance  = distance * 10 + 1;

	        }

            if (code == 72) //code equal 2
            {

                distance  = distance * 10 + 2;

            }
            if (code == 200) //code equal 3
            {

                distance  = distance * 10 + 3;

            }
            if (code == 40) //code equal 4
            {

                distance  = distance * 10 + 4;

            }
            if (code == 168) //code equal 5
            {

                distance  = distance * 10 + 5;

            }
            if (code == 104) //code equal 6
            {

                distance  = distance * 10 + 6;

            }
            if (code == 232) //code equal 7
            {

                distance  = distance * 10 + 7;

            }
            if (code == 24) //code equal 8
            {

                distance  = distance * 10 + 8;

            }
            if (code == 152) //code equal 9
            {

                distance  = distance * 10 + 9;

            }

	        if (code == 2) //Forward
	        {
	            forward(distance);
	            distance  = 0;
	        }

            if (code == 96) //Right
            {
                cw(distance);
                distance  = 0;
            }
            if (code == 224) //Left
            {
                ccw(distance);
                distance  = 0;
            }


	    }


	    //USING UART CMDS

	    putsUart0("Enter command: ");
	    getsUart0(&data);
	    parseFields(&data);

	    if (strcmp(&data.buffer[0], "forward") == 0)
        {
	        num = getFieldInteger(&data, 1);
	       forward(num);
        }
	    else if(strcmp(&data.buffer[0], "right") == 0)
        {
	        num = getFieldInteger(&data, 1);
            cw(num);
        }
	    else if(strcmp(&data.buffer[0], "left") == 0)
	    {
            num = getFieldInteger(&data, 1);
            ccw(num);
	    }




	}

}

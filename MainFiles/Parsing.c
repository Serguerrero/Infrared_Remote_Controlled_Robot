//Parse input taken from the Serial Terminal input

#include <Parsing.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"


#define MAX_CHARS 80
#define MAX_FIELDS 5


typedef struct _USER_DATA {

    char buffer[MAX_CHARS + 1];
    char fieldType[MAX_FIELDS]; //array to store the char type
    uint8_t fieldCounter;
    uint8_t fieldPosition[MAX_FIELDS];


} USER_DATA;



void parseFields(USER_DATA* data)
{


    char current;
    char last = 'd';
    bool newField = false;
    uint8_t i = 0;
    uint8_t j = 0;

    while(1)
    {
        current = data->buffer[i]; //Access the data in the buffer.
        if(current == '\0' || data->fieldCounter == MAX_FIELDS)
            return;

        //check if current char is either a Alphabetic or Numeric
        if (!newField && ((current >= 'a' && current <= 'z') ||( current >= 'A' && current <= 'Z')))
        {
            newField = true;
            data->fieldPosition[data->fieldCounter] = i;
            data->fieldType[data->fieldCounter] = 'a';
            data->fieldCounter++;

        }
        else if(!newField && (current >= '0'&& current <= '9'))
        {
            newField = true;
            data->fieldPosition[data->fieldCounter] = i;
            data->fieldType[data->fieldCounter] = 'n';
            data->fieldCounter++;
        }
        else if(!(current >= 'a' && current <= 'z') && !( current >= 'A' && current <= 'Z') && !(current >= '0' && current <= '9'))
        {

            //Code if current is a delimeter.
            data->buffer[i] = '\0';
            newField = false;
        }

        i++;
    }


    //For printing out the final string with null chars.
    for(j = 0; j < data->fieldCounter; j++)
    {
        putcUart0(data->fieldType[j]);
        putcUart0('\t');
        putsUart0(data->buffer[data->fieldPosition[j]]);
    }

}



char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{


    if ((fieldNumber <= 5) && (fieldNumber >= 0 ))
    {
        //return value of char in fieldType at fieldNumber
        return data->fieldType[fieldNumber];
    }
    else
    {
        //else if not in range return NULL
        return '/0';
    }
}


int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    int32_t r = 0;
    char* c = &data->buffer[data->fieldPosition[fieldNumber]];

    if ((fieldNumber < 5) && (fieldNumber >= 0 ) && (data->fieldType[fieldNumber] == 'n'))
    {
        uint8_t i = 0;
        while(c[i] != '\0')
        {
            r = r * 10 + (c[i] - '0');
            i++;
        }

        //return pointer to the field requested.
        return  r;
    }
    else
    {
        return 0;
    }
}


bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    // command arg1 arg2
     char* c = getFieldString(data, 0);
     if(strcmp(c, strCommand) == 0)
     {
         return true;
     }
     return false;
}



void getsUart0(USER_DATA* userData){

    int count = 0;

    //check to see if function in going to return.
    int check = 0;

    while (check == 0){

        //Getting a char from the buffer
        char c = getcUart0();

        //Pressing the backspace and count greater than 0
        if ((c == 8 || c == 127) && count > 0){
          count --;

        }
        else if(c == 13 || c == 8){

            userData->buffer[count] = '\0';
            check = 1;
        }
        else if (c >= 32)
        {
            userData->buffer[count] = c;
            count++;

            //Prints out at  80, but also prints garbage chars at the end.
            if (count == MAX_CHARS)
            {
                userData->buffer[count] = '/0';
                check = 1;
            }
        }
    }


}



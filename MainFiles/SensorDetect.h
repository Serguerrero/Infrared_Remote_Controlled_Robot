


#ifndef SENSOR_DETECT_H
#define SENSOR_DETECT_H

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
extern bool valid;
extern uint8_t code;
void initHw();
void gpioIsr();
//void setRgbColor(uint16_t red, uint16_t green, uint16_t blue);

#endif

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>


void initRgb();
void forward(uint16_t dist_cm);
void reverse(uint16_t dist_cm);
void cw(uint16_t degrees);
void ccw(uint16_t degrees);
void stop();


#endif

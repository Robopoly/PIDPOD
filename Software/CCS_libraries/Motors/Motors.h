#ifndef _MOTORS_H_
#define _MOTORS_H_

#define AIN1x	30
#define AIN2x	28
#define BIN1x	8
#define BIN2x	7
#define AISEN	23
#define BISEN	2

void motorSetup(void);
void setSpeed(int16_t, int16_t);
void TimerBaseIntHandlerA(void);
void TimerBaseIntHandlerB(void);

#endif // _MOTORS_H_

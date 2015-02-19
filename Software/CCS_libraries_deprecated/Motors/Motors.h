#ifndef _MOTORS_H_
#define _MOTORS_H_

void motorSetup(void);
void setSpeed(int16_t, int16_t);
void TimerBaseIntHandlerA(void);
void TimerBaseIntHandlerB(void);

#endif

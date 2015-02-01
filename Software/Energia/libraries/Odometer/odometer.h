#ifndef _ODOMETER_H_
#define _ODOMETER_H_

/* Initializes the odometer interrupt function */
void odometer_setup();

/* Odometer Interrupt routine */
void OdometerIntHandler(void);

#endif 

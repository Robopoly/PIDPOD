#ifndef _ODOMETER_H_
#define _ODOMETER_H_

/* Initializes the odometer interrupt function */
void odometer_setup(void);

/* Initializes the odometer controller interrupt function */
void odometer_controller_setup(void);

/* Odometer Interrupt routine */
void OdometerIntHandler(void);

/* Odometer Controller Interrupt routine */
void OdometerControllerIntHandler(void);

/* Return the upright accelerometer value corrected thanks to the odometers */
float get_accelerometer_offset(void);

#endif 

#ifndef _IMU_CONTROL_H_
#define _IMU_CONTROL_H_

/* Initializes the IMU */
void imu_setup(void);

/* Initializes the controller interrupt function */
void controller_setup();

/* Reads the IMU, but only the variables that are interesting for the segway
control loop */
void read_segway_imu(void);

/* Controller Interrupt routine */
void ControllerIntHandler(void);

#endif // _MOTORS_H_

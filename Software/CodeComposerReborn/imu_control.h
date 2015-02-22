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

/* Return upright accelerometer value */
float get_accelerometer_default(void);

/* Return speed value */
int16_t get_speed(void);

/* Set controller Parameters */
void set_controller_parameters(float p, float i, float d);

/* Checks if the pidpod is within controllable angles */
uint8_t angle_acceptable(void);

/* Return the upright value of the accelerometer. Should only be used at startup */
float get_accelerometer_default_offset(void);

float get_current1(void);

float get_current2(void);

float get_angle(void);

#endif // _MOTORS_H_

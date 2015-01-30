#ifndef _IMU_CONTROL_H_
#define _IMU_CONTROL_H_

/* Initializes the IMU */
void imu_setup(float *angle_stable, float *gyro_offset);

/* Reads the IMU, but only the variables that are interesting for the segway
control loop */
void read_segway_imu(float *acc_z, float *acc_y, float *gyro_x);

#endif // _MOTORS_H_

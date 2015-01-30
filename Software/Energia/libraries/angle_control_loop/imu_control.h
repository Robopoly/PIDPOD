#ifndef _IMU_CONTROL_H_
#define _IMU_CONTROL_H_


//Sensors conversion factors
#define ACC_RAW2MPS2 9.81/16384
#define GYRO_RAW2RADPS 3.14/180/131


void imu_setup(float *angle_stable, float *gyro_offset);



#endif // _MOTORS_H_

#include "Energia.h"
#include "imu_conrol.h"
#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include <MPU9150.h>
#include <math.h>


// bias compensation parameters
// samples to take before changing the upright position
#define NUMBER_SAMPLES 100


void imu_setup(float *angle_stable, float *gyro_offset)
{
	uint8_t i;
	float acc_z = 0;
	float acc_y = 0;
	
	*gyro_offset = 0;
  	*angle_stable = 0;
  	
  	MPU9150_init();
  	
  	// gyroscope offset
  	
  	for(i = 0; i < 16; i++)
  	{
  	  *gyro_offset += MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H);
  	  acc_z = MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H)  * ACC_RAW2MPS2;
  	  acc_y = MPU9150_readSensor(MPU9150_ACCEL_YOUT_L, MPU9150_ACCEL_YOUT_H)  * ACC_RAW2MPS2;
  	  *angle_stable += atan2(-acc_z, acc_y);
  	}
  	// average
  	*gyro_offset /= 16;
  	*angle_stable /= 16;
}
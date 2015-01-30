#include "Energia.h"
#include "imu_conrol.h"
#include "Motors.h"
#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include <MPU9150.h>
#include <math.h>


/* ---------- bias compensation parameters ------------- */
// samples to take before changing the upright position
#define NUMBER_SAMPLES 100

/* ------------ Sensors conversion factors ------------- */
#define ACC_RAW2MPS2 9.81/16384
#define GYRO_RAW2RADPS 3.14/180/131

/* --------------- Controller constants ----------------- */
#define PID_ARW 3
#define DT		4 // ?? depends on the interrupt frequency/period. Must be in microseconds, then it could be adjusted to reduce calculation time




/* ------------- Library local variables ---------------- */
float integral = 0;
float derivate = 0;
float error = 0;
float angle_stable;
float gyro_offset;
float acc_x;
float acc_z;
float gyro_x;
float angle_target;


void imu_setup(void)
{
	uint8_t i;
	float acc_z = 0;
	float acc_y = 0;
	
	gyro_offset = 0;
  	angle_stable = 0;
  	
  	MPU9150_init();
  	
  	// gyroscope offset
  	
  	for(i = 0; i < 16; i++)
  	{
  	  gyro_offset += MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H);
  	  acc_z = MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H)  * ACC_RAW2MPS2;
  	  acc_y = MPU9150_readSensor(MPU9150_ACCEL_YOUT_L, MPU9150_ACCEL_YOUT_H)  * ACC_RAW2MPS2;
  	  angle_stable += atan2(-acc_z, acc_y);
  	}
  	// average
  	gyro_offset /= 16;
  	angle_stable /= 16;
}




void controller_setup(){
	// Enable timer A peripheral
  	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
  	MAP_PRCMPeripheralReset(PRCM_TIMERA1);
  	
  	// Configure one channel for periodic interrupts, no prescaler --> 80 MHz
  	MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC);
  	MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_A, 0);
	
  	// Set timeout interrupt
  	MAP_TimerIntRegister(TIMERA1_BASE, TIMER_A, ControllerIntHandler);
  	MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);

  	// Turn on timers
  	MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, MOTOR_PRESCALER); 
  	MAP_TimerEnable(TIMERA1_BASE, TIMER_A);
}




/* Reads the IMU, but only the variables that are interesting for the segway
control loop */
void read_segway_imu(void)
{
	acc_z = MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H)  * ACC_RAW2MPS2;
  	acc_y = MPU9150_readSensor(MPU9150_ACCEL_YOUT_L, MPU9150_ACCEL_YOUT_H)  * ACC_RAW2MPS2;
  	gyro_x = (MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H) - gyro_offset) * GYRO_RAW2RADPS;
}




/* Controller Interrupt routine */
void ControllerIntHandler(void)
{
    /* Clear interrupt flag */
    HWREG(TIMERA1_BASE + TIMER_1_ICR) = 0x1;
    
    /* Get sensors */
    read_segway_imu();
  
  	/* Compute error between desired angle (angle_target) and the real angle */
  	angle = (angle + gyro_x * DT / 1000000) * 0.98 + atan2(-acc_z, acc_y) * 0.02;
  	angle_target = angle_stable;
  	error = angle_target - angle;
  	
  	/* Accumulate integral error */
  	integral = integral + error;
  	
  	/* Apply ARW protection */
  	if(integral > PID_ARW)
  	  integral = PID_ARW;
  	else if(integral < -PID_ARW)
  	  integral = -PID_ARW;

  	/* Compute controller speed */
  	speed = error * kp + integral * ki + gyro_x * kd;
  	
  	/* Apply command value to the motors (as long as the DIP4 is = 1) */
  	if(digitalRead(DIP4))
    	setSpeed(-speed, -speed);

  	else
		setSpeed(0, 0);

}

#include "Energia.h"
#include "imu_conrol.h"
#include "Motors.h"
#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include <MPU9150.h>
#include <math.h>


/* --------------- Controller constants ----------------- */
#define PID_ARW 10
#define DT		4 // ?? depends on the interrupt frequency/period. Must be in microseconds, then it could be adjusted to reduce calculation time
#define IMU_CONTROLLER_PRESCALER 8000 // depends on the period. Target period is 100Hz




/* ------------- Library local variables ---------------- */
float integral = 0;
float gyro_offset;
float accelerometer;
float upright_value_accelerometer;
float acc_reading;
float gyro_angle;



/* --------------------- Functions ----------------------- */
void imu_setup(void)
{
	uint8_t i;
	
	gyro_offset = 0;
  	accelerometer = 0;
  	
  	MPU9150_init();
  	
  	// gyroscope offset
  	
  // Find accelerometer and gyroscope offset
  float accelerometer = 0;
  gyro_offset = 0;
  for(i = 0; i < 16; i++)
  {
    accelerometer += MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H);
    gyro_offset += MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H);
  }
  // average
  upright_value_accelerometer = accelerometer/16;
  gyro_offset /= 16;
}




void controller_setup(){
	// Enable timer A peripheral
  	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
  	MAP_PRCMPeripheralReset(PRCM_TIMERA1);
  	
  	// Configure one channel for periodic interrupts, no prescaler --> 80 MHz // OR MAYBE NOT??
  	MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_A_PERIODIC);
  	MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_A, 0);
	
  	// Set timeout interrupt
  	MAP_TimerIntRegister(TIMERA1_BASE, TIMER_A, ControllerIntHandler);
  	MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);

  	// Turn on timers
  	MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, MOTOR_PRESCALER); // CHANGE HERE!!
  	MAP_TimerEnable(TIMERA1_BASE, TIMER_A);
}




/* Reads the IMU, but only the variables that are interesting for the segway
control loop */
void read_segway_imu(void)
{
	acc_reading = -(MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H) - upright_value_accelerometer) / 1000;
  	gyro_angle = (MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H) - gyro_offset) / 200;
}




/* Controller Interrupt routine */
void ControllerIntHandler(void)
{
    /* Clear interrupt flag */
    HWREG(TIMERA1_BASE + TIMER_1_ICR) = 0x1;
    
    /* Get sensors */
    read_segway_imu();
  
  	/* Compute error between desired angle (0) and the real angle */
  	angle = (angle + gyro_angle * DT / 1000000) * 0.98 + (acc_reading * 0.02);
  	
  	/* Accumulate integral error */
  	integral = integral + angle * .1;

  	/* Apply ARW protection */
  	if(integral > PID_ARW)
  	  integral = PID_ARW;
  	else if(integral < -PID_ARW)
  	  integral = -PID_ARW;

  	/* Compute controller speed */
  	speed = angle * kp + integral * ki + gyro_angle * kd;
  	
  	/* Apply command value to the motors (as long as the DIP4 is = 1) */
  	if(digitalRead(DIP4))
    	setSpeed(-speed, -speed);

  	else
		setSpeed(0, 0);

}

#include <math.h>
#include <stdint.h>

#include "imu_control.h"
#include "pidpod_gpio.h"
#include "Motors.h"
#include "odometer.h"

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_timer.h"
#include "rom_map.h"
#include "prcm.h"
#include "timer.h"
#include "MPU9150.h"


/* --------------- Controller constants ----------------- */
#define PID_ARW 					4.
#define DT							10000. 	// ?? depends on the interrupt frequency/period. Must be in microseconds, then it could be adjusted to reduce calculation time
#define IMU_CONTROLLER_STARTUP	 	8000 	// depends on the period. Target period is 100Hz
#define IMU_CONTROLLER_PRESCALER	100   	// clock frequency is now 800KHz

/* Other constants */
#define DIP4 15


/* ------------- Library local variables ---------------- */
float integral = 0;
float gyro_offset;
float accelerometer;
float upright_value_accelerometer;
float acc_reading;
float gyro_angle;
float angle = 0;
int16_t speed = 0;
float Kp,Ki,Kd;	// again, why not recognized? energia/arduino weird stuff? They should be GLOBAL! :/ Still throws an error during compilation if ki,kp,kd instead of Kp,Ki,Kd





/* --------------------- Functions ----------------------- */
void imu_setup(void)
{
	uint8_t i;


	MPU9150_init();

	// gyroscope offset

	// Find accelerometer and gyroscope offset
	accelerometer = 0;
	gyro_offset = 0;
	for(i = 0; i < 16; i++)
	{
	accelerometer += MPU9150_readSensor_2byte(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H);
	gyro_offset += MPU9150_readSensor_2byte(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H);
	}
	// average
	upright_value_accelerometer = accelerometer/16;
	gyro_offset /= 16;
}




void controller_setup(){
	// Enable timer A peripheral
  	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
  	MAP_PRCMPeripheralReset(PRCM_TIMERA1);
  	
  	// Configure one channel for periodic interrupts
  	MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
  	MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_A, IMU_CONTROLLER_PRESCALER);
	
  	// Set timeout interrupt
  	MAP_TimerIntRegister(TIMERA1_BASE, TIMER_A, ControllerIntHandler);
  	MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);

  	// Turn on timers
  	MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, IMU_CONTROLLER_STARTUP); 
  	MAP_TimerEnable(TIMERA1_BASE, TIMER_A);
}




/* Reads the IMU, but only the variables that are interesting for the segway
control loop */
void read_segway_imu(void)
{
	upright_value_accelerometer = get_accelerometer_offset();
	acc_reading = -(MPU9150_readSensor_2byte(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H) - upright_value_accelerometer) / 1000;
  	gyro_angle = (MPU9150_readSensor_2byte(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H) - gyro_offset) / 200;
}




/* Controller Interrupt routine */
void ControllerIntHandler(void)
{
    /* Clear interrupt flag */
    HWREG(TIMERA1_BASE + TIMER_O_ICR) = 0x1; 
    
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
  	speed = angle * Kp + integral * Ki + gyro_angle * Kd;
  	
  	
  	
  	/* Apply command value to the motors (as long as the DIP4 is = 1) */
  	//if(readDIP4() && angle_acceptable())
  	if(readDIP4())
    	setSpeed(speed, speed);

  	else
		setSpeed(0, 0);
}



float get_accelerometer_default(void)
{
	return accelerometer;
}

int16_t get_speed(void)
{
	return  speed;
}

void set_controller_parameters(float p, float i, float d)
{
	Kp = p;
	Ki = i;
	Kd = d;
}

uint8_t angle_acceptable(void)
{
	static uint8_t unstable_count;
	
	/* Count how many times the angle of PIDPOD was outside stabilizable range */
	if((angle > -6) && (angle < 15))
		unstable_count = 0;
	else 
		unstable_count++;
		
	if(unstable_count > 250) // avoid overflow
		unstable_count = 250;
	
	/* If for more than 0.5 seconds, return 0*/	
	if(unstable_count > 50)
		return 0;
	else
		return 1;
}

float get_accelerometer_default_offset(void)
{
	return upright_value_accelerometer;
}



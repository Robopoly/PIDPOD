#include <math.h>
#include <stdint.h>

#include "odometer.h"
#include "pidpod_gpio.h"
#include "imu_control.h"
#include "hw_types.h"
#include "hw_timer.h"
#include "prcm.h"
#include "timer.h"
#include "hw_memmap.h"
#include "rom_map.h"



/* --------------------- Constants ---------------------- */
#define ODOMETER_STARTUP	 			4000 	// depends on the period. Target period is 400Hz
#define ODOMETER_PRESCALER				50  	// clock frequency is now 1.6 MHz			
#define ODOMETER_CONTROLLER_STARTUP	 	64000 	// depends on the period. Target period is 10Hz (5Hz)
#define ODOMETER_CONTROLLER_PRESCALER	250   	// clock frequency is now 320 KHz
float Ki_odo,  Kp_odo,Kd_odo;
const float ODO_ARW		= 100;
const float ODO_STEPS = 36;
const float WHEEL_DIAMETER = 18; // more or less, in cm
const float DISTANCE_MULTIPLIER = 0.5; // 18/36 = 0.5

/* ------------------ Pin definitions ------------------- */
#define ODO1	14 
#define ODO2	6



/* ------------- Library local variables ---------------- */
uint8_t odo1 = 0;
uint8_t odo2 = 0;
uint8_t odo1_old = 0;
uint8_t odo2_old = 0;
int32_t odo1_total = 0;
int32_t odo2_total = 0;
int32_t odo1_total_old = 0;
int32_t odo2_total_old = 0;
int32_t distance_delta1 = 0;
int32_t distance_delta2 = 0;
int32_t acceleration = 0;
float acc_value = 0;
float acc_value_startup = 0;


/* --------------------- Functions ----------------------- */


void odometer_setup(void){

	acc_value_startup = get_accelerometer_default_offset(); // because strange bug if in previous position..

	// Enable timer A peripheral
  	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
  	MAP_PRCMPeripheralReset(PRCM_TIMERA2);
  	
  	// Configure one channel for periodic interrupts 
  	MAP_TimerConfigure(TIMERA2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
  	MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_A, ODOMETER_PRESCALER);
	
  	// Set timeout interrupt
  	MAP_TimerIntRegister(TIMERA2_BASE, TIMER_A, OdometerIntHandler);
  	MAP_TimerIntEnable(TIMERA2_BASE, TIMER_TIMA_TIMEOUT);

  	// Turn on timers
  	MAP_TimerLoadSet(TIMERA2_BASE, TIMER_A, ODOMETER_STARTUP); 
  	MAP_TimerEnable(TIMERA2_BASE, TIMER_A);
}

void odometer_controller_setup(void){

	//acc_value_startup = get_accelerometer_default_offset();

	// Enable timer A peripheral
  	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);
  	MAP_PRCMPeripheralReset(PRCM_TIMERA3);
  	
  	// Configure one channel for periodic interrupts 
  	MAP_TimerConfigure(TIMERA3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
  	MAP_TimerPrescaleSet(TIMERA3_BASE, TIMER_A, ODOMETER_CONTROLLER_PRESCALER);
	
  	// Set timeout interrupt
  	MAP_TimerIntRegister(TIMERA3_BASE, TIMER_A, OdometerControllerIntHandler);
  	MAP_TimerIntEnable(TIMERA3_BASE, TIMER_TIMA_TIMEOUT);

  	// Turn on timers
  	MAP_TimerLoadSet(TIMERA3_BASE, TIMER_A, ODOMETER_CONTROLLER_STARTUP); 
  	MAP_TimerEnable(TIMERA3_BASE, TIMER_A);
}

/* Odometer Interrupt routine */
void OdometerIntHandler(void)												
{
	int16_t speed;
	speed = get_speed();
	
    /* Clear interrupt flag */
    HWREG(TIMERA2_BASE + TIMER_O_ICR) = 0x1; 
    
	/* Read odometers */
	odo1 = readODO1();
	odo2 = readODO2();
	
	/* Keep track of the odometry depending on the motor speed*/
	if(odo1 != odo1_old)
	{
		if(speed >= 0)
			odo1_total++;
		else
			odo1_total--;
	}
	
	if(odo2 != odo2_old)
	{
		if(speed >= 0)
			odo2_total++;
		else
			odo2_total--;
	}
		
	odo1_old = odo1;
	odo2_old = odo2;

}

/* Odometer controller Interrupt routine */
void OdometerControllerIntHandler(void)												
{	
	static uint8_t debug;
	static int32_t distance_delta_old;

    /* Clear interrupt flag */
    HWREG(TIMERA3_BASE + TIMER_O_ICR) = 0x1; 
    
    /* Compute advance between two steps */
	distance_delta1 = odo1_total - odo1_total_old;
	odo1_total_old = odo1_total;

	/* Compute acceleration */
	acceleration = distance_delta1 - distance_delta_old;
	distance_delta_old = distance_delta1;

	/* Here PID controller for horizontal movement */
	acc_value = acc_value_startup + (float)distance_delta1 * Kp_odo + (float)odo1_total * Ki_odo + (float)acceleration * Kd_odo;

	
	/* ARW for position */
	if(odo1_total > ODO_ARW)
	{
		odo1_total = ODO_ARW;
		odo1_total_old = ODO_ARW;
	}
	if(odo1_total < -ODO_ARW)
	{
		odo1_total = -ODO_ARW;
		odo1_total_old = -ODO_ARW;
	}

	/* LED blink */
	if(debug)
	{
		setBLED();
		debug = 0;
	}
	else
	{
		clearBLED();
		debug = 1;
	}
}

float get_accelerometer_offset(void)
{
	return acc_value;
}

void set__odo_controller_parameters(float p, float i, float d)
{
	Kp_odo = p;
	Ki_odo = i;
	Kd_odo = d;
}

float get_odometry(void)
{
	return ((float)odo2_total * DISTANCE_MULTIPLIER);
}



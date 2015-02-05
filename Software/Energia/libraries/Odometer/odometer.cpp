#include "Energia.h"
#include "odometer.h"
#include "imu_control.h"
#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include <math.h>


/* --------------------- Constants ---------------------- */
#define ODOMETER_STARTUP	 			4000 	// depends on the period. Target period is 400Hz
#define ODOMETER_PRESCALER				50  	// clock frequency is now 1.6 MHz			
#define ODOMETER_CONTROLLER_STARTUP	 	32000 	// depends on the period. Target period is 10Hz
#define ODOMETER_CONTROLLER_PRESCALER	250   	// clock frequency is now 320 KHz	
#define KI_ODOMETER						3.		// 1 worked quite well
#define KP_ODOMETER						500.		// 4 worked quite well
#define KD_ODOMETER						40.		// 
#define ODO_ARW							15

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
float acc_value_startup;



/* --------------------- Functions ----------------------- */


void odometer_setup(void){														
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
	// gets variable
	acc_value_startup = get_accelerometer_default_offset();
													
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
	odo1 = digitalRead(ODO1);
	odo2 = digitalRead(ODO2);
	
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
	static uint8_t i;
	static uint8_t allow;
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
	acc_value = acc_value_startup + (float)distance_delta1 * KP_ODOMETER + (float)odo1_total * KI_ODOMETER + (float)acceleration * KD_ODOMETER;

	
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
		digitalWrite(5,HIGH);
		debug = 0;
	}
	else
	{
		digitalWrite(5,LOW);
		debug = 1;
	}

}

float get_accelerometer_offset(void)
{
	return acc_value;
}

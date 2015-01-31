#include "Energia.h"
#include "odometer.h"
#include "imu_control.h"
#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include <math.h>


/* --------------------- Constants ---------------------- */
#define ODOMETER_STARTUP	 	8000 // depends on the period. Target period is 100Hz
#define ODOMETER_PRESCALER	100   // clock frequency is now 800KHz			

																		// VERIFY HERE 1

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


/* --------------------- Functions ----------------------- */


void odometer_setup(){														// VERIFY HERE 2
	// Enable timer A peripheral
  	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
  	MAP_PRCMPeripheralReset(PRCM_TIMERA2);
  	
  	// Configure one channel for periodic interrupts, no prescaler --> 80 MHz // OR MAYBE NOT??
  	MAP_TimerConfigure(TIMERA2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
  	MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_A, ODOMETER_PRESCALER);
	
  	// Set timeout interrupt
  	MAP_TimerIntRegister(TIMERA2_BASE, TIMER_A, OdometerIntHandler);
  	MAP_TimerIntEnable(TIMERA2_BASE, TIMER_TIMA_TIMEOUT);

  	// Turn on timers
  	MAP_TimerLoadSet(TIMERA2_BASE, TIMER_A, ODOMETER_STARTUP); 
  	MAP_TimerEnable(TIMERA2_BASE, TIMER_A);
}

/* Odometer Interrupt routine */
void OdometerIntHandler(void)												// VERIFY HERE 3 + the header
{
	int8_t speed;
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

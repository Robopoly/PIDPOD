/* Standard include */
#include <stdint.h>
#include <math.h>

#include "Motors.h"

/* Driverlib includes */
#include "hw_types.h"
#include "hw_timer.h"
#include "hw_ints.h"
#include "prcm.h"
#include "timer.h"
#include "pin.h"
#include "hw_memmap.h"
#include "rom_map.h"
#include "gpio.h"
#include "hw_gpio.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "timer.h"
#include "utils.h"

/* Common interface includes */
#include "timer_if.h"
#include "gpio_if.h"

/* GLOBALS */
unsigned int port_ain1, port_ain2, port_bin1, port_bin2;
unsigned char pin_ain1, pin_ain2, pin_bin1, pin_bin2;
uint8_t bitA1, bitA2, bitB1, bitB2;

int8_t speedLeft = 0;
int8_t speedRight = 0;

/* DEFINES */

#define AIN1	PIN_50
#define AIN2	PIN_53
//#define BIN1	PIN_63 // NEW VERSION
#define BIN1	PIN_62
#define BIN2	PIN_05
#define AIN1x	0
#define AIN2x	30
// #define BIN1x	8 // new version
#define BIN1x	7
#define BIN2x	14
#define MOTOR_PRESCALER 8000
#define MOTOR_OFFSET 400




void motorSetup()
{
	/* Set pin mode for Hbridge output pins */
	MAP_PinTypeGPIO(AIN1, PIN_MODE_0, false); /* Ain 1 */
	MAP_PinTypeGPIO(AIN2, PIN_MODE_0, false); /* Bin 1 */
	MAP_PinTypeGPIO(BIN1, PIN_MODE_0, false);  /* Bin 2 */
	MAP_PinTypeGPIO(BIN2, PIN_MODE_0, false); /* Ain 2 */

	/* Get port name and bin number from GPIO number (TI lookup table) */
	GPIO_IF_GetPortNPin(AIN1x, &port_ain1, &pin_ain1);
	GPIO_IF_GetPortNPin(AIN2x, &port_ain2, &pin_ain2);
	GPIO_IF_GetPortNPin(BIN1x, &port_bin1, &pin_bin1);
	GPIO_IF_GetPortNPin(BIN2x, &port_bin2, &pin_bin2);

	/* Set pin direction */
	GPIODirModeSet(port_ain1, pin_ain1, 1);
	GPIODirModeSet(port_ain2, pin_ain2, 1);
	GPIODirModeSet(port_bin1, pin_bin1, 1);
	GPIODirModeSet(port_bin2, pin_bin2, 1);

	/* Set value to write to PIN */
	bitA1 = 1 << (AIN1x % 8);
	bitA2 = 1 << (AIN2x % 8);
	bitB1 = 1 << (BIN1x % 8);
	bitB2 = 1 << (BIN2x % 8);

	// Enable timer A peripheral
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralReset(PRCM_TIMERA0);

	// Split channels and configure for periodic interrupts
	MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC);
	MAP_TimerPrescaleSet(TIMERA0_BASE, TIMER_A, 0);
	MAP_TimerPrescaleSet(TIMERA0_BASE, TIMER_B, 0);

	// Set compare interrupt
	MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_MATCH);
	MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMB_MATCH);

	// Configure compare interrupt, start with 0 speed
	MAP_TimerMatchSet(TIMERA0_BASE, TIMER_A, 0);
	MAP_TimerMatchSet(TIMERA0_BASE, TIMER_B, 0);

	// Set timeout interrupt
	MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, TimerBaseIntHandlerA);
	MAP_TimerIntRegister(TIMERA0_BASE, TIMER_B, TimerBaseIntHandlerB);
	MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
	MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMB_TIMEOUT);

	// Turn on timers
	MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, MOTOR_PRESCALER);
	MAP_TimerLoadSet(TIMERA0_BASE, TIMER_B, MOTOR_PRESCALER);

	MAP_TimerEnable(TIMERA0_BASE, TIMER_A);
	MAP_TimerEnable(TIMERA0_BASE, TIMER_B);
}

void TimerBaseIntHandlerA(void)
{
	if(HWREG(TIMERA0_BASE + TIMER_O_MIS) & 0x10)
	{
		// Match interrupt
		HWREG(TIMERA0_BASE + TIMER_O_ICR) = 0x10;
		if(speedLeft > 0)
		{
			MAP_GPIOPinWrite(port_ain1, pin_ain1, 0);
			MAP_GPIOPinWrite(port_ain2, pin_ain2, bitA2);
		}
		else
		{
			MAP_GPIOPinWrite(port_ain1, pin_ain1, bitA1);
			MAP_GPIOPinWrite(port_ain2, pin_ain2, 0);
		}
	}
	else
	{
		// Overflow interrupt
		HWREG(TIMERA0_BASE + TIMER_O_ICR) = 0x1;
		MAP_GPIOPinWrite(port_ain1, pin_ain1, bitA1);
		MAP_GPIOPinWrite(port_ain2, pin_ain2, bitA2);
	}
}

void TimerBaseIntHandlerB(void)
{
	if(HWREG(TIMERA0_BASE + TIMER_O_MIS) & 0x800)
	{
		// Match interrupt
		HWREG(TIMERA0_BASE + TIMER_O_ICR) = 0x800;
		if(speedRight > 0)
		{
			MAP_GPIOPinWrite(port_bin1, pin_bin1, 0);
			MAP_GPIOPinWrite(port_bin2, pin_bin2, bitB2);
		}
		else
		{
			MAP_GPIOPinWrite(port_bin1, pin_bin1, bitB1);
			MAP_GPIOPinWrite(port_bin2, pin_bin2, 0);
		}
	}
	else
	{
	  // Overflow interrupt
	  HWREG(TIMERA0_BASE + TIMER_O_ICR) = 0x100;

	  MAP_GPIOPinWrite(port_bin1, pin_bin1, bitB1);
	  MAP_GPIOPinWrite(port_bin2, pin_bin2, bitB2);
	}
}

void setSpeed(int16_t left, int16_t right)
{
	if(left > 100)
	{
		speedLeft = 100;
	}
	else if(left < -100)
	{
		speedLeft = -100;
	}
	else
	{
		speedLeft = left;
	}

	if(right > 100)
	{
		speedRight = 100;
	}
	else if(right < -100)
	{
		speedRight = -100;
	}
	else
	{
		speedRight = right;
	}

	// set interrupt call speed
	uint32_t interruptLeft = (MOTOR_PRESCALER - MOTOR_OFFSET) * abs(speedLeft) / 100;
	uint32_t interruptRight = (MOTOR_PRESCALER - MOTOR_OFFSET) * abs(speedRight) / 100;

	MAP_TimerMatchSet(TIMERA0_BASE, TIMER_A, interruptLeft);
	MAP_TimerMatchSet(TIMERA0_BASE, TIMER_B, interruptRight);
}

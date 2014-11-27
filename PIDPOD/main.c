// Standard include
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "interrupt.h"
#include "hw_ints.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "timer.h"
#include "utils.h"
#include "gpio.h"

// Common interface includes
#include "timer_if.h"
#include "gpio_if.h"

/* DEFINES */

#define AIN1	0
#define AIN2	28
#define BIN1	8
#define BIN2	14


/* GLOBALS */
unsigned int port_ain1, port_ain2, port_bin1, port_bin2;
unsigned char pin_ain1, pin_ain2, pin_bin1, pin_bin2;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif

//*****************************************************************************
//
// Globals used by the timer interrupt handler.
//
//*****************************************************************************
static volatile unsigned long g_ulSysTickValue;
static volatile unsigned long g_ulBase;
static volatile unsigned long g_ulRefBase;
static volatile unsigned long g_ulRefTimerInts = 0;
static volatile unsigned long g_ulIntClearVector;
unsigned long g_ulTimerInts;

static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


void TimerBaseIntHandlerA(void)
{
	/* Match interrupt */
	if(TimerIntStatus(TIMERA0_BASE,false) & 0x10)
	{
		TimerIntClear(TIMERA0_BASE, 0x10);
		GPIOPinWrite(port_ain1, pin_ain1, 0);
		GPIOPinWrite(port_ain2, pin_ain2, 0);
	}

	/* Overflow interrupt */
	else
	{
		TimerIntClear(TIMERA0_BASE, 0x1);
		GPIOPinWrite(port_ain1, pin_ain1, 0);
		GPIOPinWrite(port_ain2, pin_ain2, 1);
	}


}

void TimerBaseIntHandlerB(void)
{

	/* Match interrupt */
	if(TimerIntStatus(TIMERA0_BASE,false) & 0x800)
	{
		TimerIntClear(TIMERA0_BASE, 0x800);
		GPIOPinWrite(port_bin1, pin_bin1, 0);
		GPIOPinWrite(port_bin2, pin_bin2, 0);
	}


	/* Overflow interrupt */
	else
	{
		TimerIntClear(TIMERA0_BASE, 0x100);
		GPIOPinWrite(port_bin1, pin_bin1, 0);
		GPIOPinWrite(port_bin2, pin_bin2, 1);
	}


}


void InitTimer(void)
{
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_BOTH, 0);

    /* Set timeout interrupt */
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerBaseIntHandlerA);
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_B, TimerBaseIntHandlerB);

    /* Set compare interrupt */
    TimerIntEnable(TIMERA0_BASE,TIMER_CAPA_MATCH);
    TimerIntEnable(TIMERA0_BASE,TIMER_CAPB_MATCH);

    /* Configure compare interrupt */
    TimerMatchSet(TIMERA0_BASE, TIMER_A, 0);
    TimerMatchSet(TIMERA0_BASE, TIMER_B, 0);

    /* Turn on timers */
    Timer_IF_Start(TIMERA0_BASE, TIMER_A, 4000);
    Timer_IF_Start(TIMERA0_BASE, TIMER_B, 4000);
}

void InitGPIO(void)
{
	GPIO_IF_GetPortNPin(AIN1, &port_ain1, &pin_ain1);
	GPIO_IF_GetPortNPin(AIN2, &port_ain2, &pin_ain2);
	GPIO_IF_GetPortNPin(BIN1, &port_bin1, &pin_bin1);
	GPIO_IF_GetPortNPin(BIN2, &port_bin2, &pin_bin2);

	GPIODirModeSet(port_ain1, pin_ain1, 1);
	GPIODirModeSet(port_ain2, pin_ain2, 1);
	GPIODirModeSet(port_bin1, pin_bin1, 1);
	GPIODirModeSet(port_bin2, pin_bin2, 1);
}

void setSpeed(int a, int b)
{
	// TODO : must process a and b
	TimerMatchSet(TIMERA0_BASE, TIMER_A, a);
	TimerMatchSet(TIMERA0_BASE, TIMER_B, b);
}

int main(void)
{
	BoardInit();
	InitTimer();
	InitGPIO();
	setSpeed(2000,2000);
	

	while(1);
	//return 0;
}

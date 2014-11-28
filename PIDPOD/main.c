/* Standard include */
#include <stdio.h>

/* Driverlib includes */
#include "hw_types.h"
#include "hw_timer.h"
#include "pin.h"
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

/* Common interface includes */
#include "timer_if.h"
#include "gpio_if.h"

/* DEFINES */

#define AIN1	0
#define AIN2	30
#define BIN1	8
#define BIN2	14


/* GLOBALS */
unsigned int port_ain1, port_ain2, port_bin1, port_bin2;
unsigned char pin_ain1, pin_ain2, pin_bin1, pin_bin2;

/* Globals used by the timer interrupt handler */
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif


static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS

#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


void TimerBaseIntHandlerA(void)
{
	/* Match interrupt */
	if(TimerIntStatus(TIMERA0_BASE,true) & 0x10)
	{
		TimerIntClear(TIMERA0_BASE, 0x10);
		GPIO_IF_Set(AIN1, port_ain1, pin_ain1, 0);
		GPIO_IF_Set(AIN2, port_ain2, pin_ain2, 0);
	}

	/* Overflow interrupt */
	else
	{
		TimerIntClear(TIMERA0_BASE, 0x1);
		GPIO_IF_Set(AIN1, port_ain1, pin_ain1, 0);
		GPIO_IF_Set(AIN2, port_ain2, pin_ain2, 1);
	}
}

void TimerBaseIntHandlerB(void)
{

	/* Match interrupt */
	if(TimerIntStatus(TIMERA0_BASE,true) & 0x800)
	{
		TimerIntClear(TIMERA0_BASE, 0x800);
		GPIO_IF_Set(BIN1, port_bin1, pin_bin1, 0);
		GPIO_IF_Set(BIN2, port_bin2, pin_bin2, 0);
	}

	/* Overflow interrupt */
	else
	{
		TimerIntClear(TIMERA0_BASE, 0x100);
		GPIO_IF_Set(BIN1, port_bin1, pin_bin1, 0);
		GPIO_IF_Set(BIN2, port_bin2, pin_bin2, 1);
	}
}


void InitTimer(void)
{
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC, TIMER_A, 0);

    /* Set compare interrupt */
    TimerIntEnable(TIMERA0_BASE,TIMER_TIMA_MATCH);
    TimerIntEnable(TIMERA0_BASE,TIMER_TIMB_MATCH);

    /* Configure compare interrupt, start with 0 speed */
    TimerMatchSet(TIMERA0_BASE, TIMER_A, 4000);
    TimerMatchSet(TIMERA0_BASE, TIMER_B, 4000);

    /* Set timeout interrupt */
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerBaseIntHandlerA);
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_B, TimerBaseIntHandlerB);

    /* Turn on timers */
    Timer_IF_Start(TIMERA0_BASE, TIMER_A, 4000);
    Timer_IF_Start(TIMERA0_BASE, TIMER_B, 4000);
}

void InitGPIO(void)
{
	/* Activate GPIO clock per port. Not sure of which ports we use, so enable everything */
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA4, PRCM_RUN_MODE_CLK);

	/* No idea what this is for */
	MAP_PinTypeGPIO(PIN_50, PIN_MODE_0, false); /* Ain 1 */
	MAP_PinTypeGPIO(PIN_63, PIN_MODE_0, false); /* Bin 1 */
	MAP_PinTypeGPIO(PIN_05, PIN_MODE_0, false);  /* Bin 2 */
	MAP_PinTypeGPIO(PIN_53, PIN_MODE_0, false); /* Ain 2 */

	/* Get port name and bin number from GPIO number (TI lookup table) */
	GPIO_IF_GetPortNPin(AIN1, &port_ain1, &pin_ain1);
	GPIO_IF_GetPortNPin(AIN2, &port_ain2, &pin_ain2);
	GPIO_IF_GetPortNPin(BIN1, &port_bin1, &pin_bin1);
	GPIO_IF_GetPortNPin(BIN2, &port_bin2, &pin_bin2);

	/* Set pin direction */
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

#include <stdint.h>

#include "pidpod_gpio.h"

/* Driverlib includes */
#include "hw_types.h"
#include "hw_timer.h"
#include "prcm.h"
#include "pin.h"
#include "hw_memmap.h"
#include "rom_map.h"
#include "gpio.h"
#include "hw_gpio.h"

/* Common interface includes */
#include "gpio_if.h"

/* Gloab variables */
unsigned char pin_rled, pin_bled, pin_dip1, pin_dip2, pin_dip3, pin_dip4, pin_odo1, pin_odo2;
unsigned int port_rled, port_bled, port_dip1, port_dip2, port_dip3, port_dip4, port_odo1, port_odo2;

/* Pin definitions */
#define RLED    PIN_64
#define BLED	PIN_61
#define DIP4    PIN_18
#define DIP3    PIN_08
#define DIP2    PIN_45
#define DIP1    PIN_07
#define ODO1    PIN_06
#define ODO2    PIN_59
#define RLEDx   9
#define BLEDx	6
#define DIP4x   28
#define DIP3x   17
#define DIP2x   31
#define DIP1x   16
#define ODO1x   15
#define ODO2x   4

void InitGPIO(void)
{
	/* Activate GPIO clock per port. Not sure of which ports we use, so enable everything */
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA4, PRCM_RUN_MODE_CLK);

    /* Configure UART pins */
    MAP_PinTypeUART(PIN_55, PIN_MODE_3);
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);

    /* Configure I2C pins */
    MAP_PinTypeI2C(PIN_01, PIN_MODE_1);
    MAP_PinTypeI2C(PIN_02, PIN_MODE_1);

	/* Set pin mode for DIP and LED pins */
	MAP_PinTypeGPIO(RLED, PIN_MODE_0, false);
	MAP_PinTypeGPIO(BLED, PIN_MODE_0, false);
	MAP_PinTypeGPIO(DIP1, PIN_MODE_0, false);
	MAP_PinTypeGPIO(DIP2, PIN_MODE_0, false);
	MAP_PinTypeGPIO(DIP3, PIN_MODE_0, false);
	MAP_PinTypeGPIO(DIP4, PIN_MODE_0, false);
	MAP_PinTypeGPIO(ODO1, PIN_MODE_0, false);
	MAP_PinTypeGPIO(ODO2, PIN_MODE_0, false);

	/* Get port name and bin number from GPIO number (TI lookup table) LED, DIP, ODO */
	GPIO_IF_GetPortNPin(RLEDx, &port_rled, &pin_rled);
	GPIO_IF_GetPortNPin(BLEDx, &port_bled, &pin_bled);
	GPIO_IF_GetPortNPin(DIP1x, &port_dip1, &pin_dip1);
	GPIO_IF_GetPortNPin(DIP2x, &port_dip2, &pin_dip2);
	GPIO_IF_GetPortNPin(DIP3x, &port_dip3, &pin_dip3);
	GPIO_IF_GetPortNPin(DIP4x, &port_dip4, &pin_dip4);
	GPIO_IF_GetPortNPin(ODO1x, &port_odo1, &pin_odo1);
	GPIO_IF_GetPortNPin(ODO2x, &port_odo2, &pin_odo2);

	/* Set pin direction LED, DIP, ODO */
	GPIODirModeSet(port_rled, pin_rled, 1);
	GPIODirModeSet(port_bled, pin_bled, 1);
	GPIODirModeSet(port_dip1, pin_dip1, 0);
	GPIODirModeSet(port_dip2, pin_dip2, 0);
	GPIODirModeSet(port_dip3, pin_dip3, 0);
	GPIODirModeSet(port_dip4, pin_dip4, 0);
	GPIODirModeSet(port_odo1, pin_odo1, 0);
	GPIODirModeSet(port_odo2, pin_odo2, 0);
}

uint8_t readDIP1(void)
{
	return (uint8_t)GPIO_IF_Get(DIP1x, port_dip1, pin_dip1);
}

uint8_t readDIP2(void)
{
	return (uint8_t)GPIO_IF_Get(DIP2x, port_dip2, pin_dip2);
}

uint8_t readDIP3(void)
{
	return (uint8_t)GPIO_IF_Get(DIP3x, port_dip3, pin_dip3);
}

uint8_t readDIP4(void)
{
	return (uint8_t)GPIO_IF_Get(DIP4x, port_dip4, pin_dip4);
}

uint8_t readODO1(void)
{
	return (uint8_t)GPIO_IF_Get(ODO1x, port_odo1, pin_odo1);
}

uint8_t readODO2(void)
{
	return (uint8_t)GPIO_IF_Get(ODO2x, port_odo2, pin_odo2);
}

void setRLED(void)
{
	GPIO_IF_Set(RLEDx, port_rled, pin_rled, 1);
}

void clearRLED(void)
{
	GPIO_IF_Set(RLEDx, port_rled, pin_rled, 0);
}

void setBLED(void)
{
	GPIO_IF_Set(BLEDx, port_bled, pin_bled, 1);
}

void clearBLED(void)
{
	GPIO_IF_Set(BLEDx, port_bled, pin_bled, 0);
}

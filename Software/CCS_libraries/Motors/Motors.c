#include "Energia.h"
#include "Motors.h"
#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>

#define MOTOR_PRESCALER 8000
#define MOTOR_OFFSET 400

int8_t speedLeft = 0;
int8_t speedRight = 0;

uint8_t bitA1, bitA2, bitB1, bitB2;
uint8_t portA1, portA2, portB1, portB2;
uint32_t baseA1, baseA2, baseB1, baseB2;

void motorSetup()
{
  pinMode(AIN1x, OUTPUT);
  pinMode(AIN2x, OUTPUT);
  pinMode(BIN1x, OUTPUT);
  pinMode(BIN2x, OUTPUT);

  bitA1 = digitalPinToBitMask(AIN1x);
  bitA2 = digitalPinToBitMask(AIN2x);
  bitB1 = digitalPinToBitMask(BIN1x);
  bitB2 = digitalPinToBitMask(BIN2x);
  
  portA1 = digitalPinToPort(AIN1x);
  portA2 = digitalPinToPort(AIN2x);
  portB1 = digitalPinToPort(BIN1x);
  portB2 = digitalPinToPort(BIN2x);
  
  baseA1 = (uint32_t) portBASERegister(portA1);
  baseA2 = (uint32_t) portBASERegister(portA2);
  baseB1 = (uint32_t) portBASERegister(portB1);
  baseB2 = (uint32_t) portBASERegister(portB2);
  
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
      MAP_GPIOPinWrite(baseA1, bitA1, 0);
      MAP_GPIOPinWrite(baseA2, bitA2, bitA2);
    }
    else
    {
      MAP_GPIOPinWrite(baseA1, bitA1, bitA1);
      MAP_GPIOPinWrite(baseA2, bitA2, 0);
    }
  }
  else
  {
    // Overflow interrupt
    HWREG(TIMERA0_BASE + TIMER_O_ICR) = 0x1;
    MAP_GPIOPinWrite(baseA1, bitA1, bitA1);
    MAP_GPIOPinWrite(baseA2, bitA2, bitA2);
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
      MAP_GPIOPinWrite(baseB1, bitB1, 0);
      MAP_GPIOPinWrite(baseB2, bitB2, bitB2);
    }
    else
    {
      MAP_GPIOPinWrite(baseB1, bitB1, bitB1);
      MAP_GPIOPinWrite(baseB2, bitB2, 0);
    }
  }
  else
  {
    // Overflow interrupt
    HWREG(TIMERA0_BASE + TIMER_O_ICR) = 0x100;
    
    MAP_GPIOPinWrite(baseB1, bitB1, bitB1);
    MAP_GPIOPinWrite(baseB2, bitB2, bitB2);
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
/*
 * Name:        PIDPOD
 * Date:        2014-11-28
 * Version:     1.0
 * Description: Segway-type self-balancing robot sketch based on the CC3200 development board.
 */

#include <inc/hw_types.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
//#include <driverlib/adc.h>

// Includes for BMA222 accelerometer communication
#include <Wire.h>
#include <BMA222.h>

// PID coefficients
#define UPRIGHT_VALUE_ACCELEROMETER 0
#define KP 1.
#define KI 0.
#define KD 0.

BMA222 accelerometer;

// Pin definitions
#define LED RED_LED
#define AIN1	30
#define AIN2	28
#define BIN1	27
#define BIN2	7

void motorSetup()
{
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  
  // Enable timer A peripheral
  MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
  MAP_PRCMPeripheralReset(PRCM_TIMERA0);
  
  // Split channels and configure for periodic interrupts
  MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC);

  // Set compare interrupt
  MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_MATCH);
  MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMB_MATCH);

  // Configure compare interrupt, start with 0 speed
  MAP_TimerMatchSet(TIMERA0_BASE, TIMER_A, 4000);
  MAP_TimerMatchSet(TIMERA0_BASE, TIMER_B, 4000);

  // Set timeout interrupt
  MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, TimerBaseIntHandlerA);
  MAP_TimerIntRegister(TIMERA0_BASE, TIMER_B, TimerBaseIntHandlerB);
  MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);

  // Turn on timers
  MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, 4000);
  MAP_TimerLoadSet(TIMERA0_BASE, TIMER_B, 4000);
 
  MAP_TimerEnable(TIMERA0_BASE, TIMER_A);
  MAP_TimerEnable(TIMERA0_BASE, TIMER_B);
}

void TimerBaseIntHandlerA(void)
{
  if(TimerIntStatus(TIMERA0_BASE, true) & 0x10)
  {
    // Match interrupt
    TimerIntClear(TIMERA0_BASE, 0x10);
    digitalWrite(AIN1, 0);
    digitalWrite(AIN2, 0);
  }
  else
  {
    // Overflow interrupt
    TimerIntClear(TIMERA0_BASE, 0x1);
    digitalWrite(AIN1, 0);
    digitalWrite(AIN2, 1);
  }
}

void TimerBaseIntHandlerB(void)
{
  if(TimerIntStatus(TIMERA0_BASE, true) & 0x800)
  {
    // Match interrupt
    TimerIntClear(TIMERA0_BASE, 0x800);
    digitalWrite(BIN1, 0);
    digitalWrite(BIN2, 0);
  }
  else
  {
    // Overflow interrupt
    TimerIntClear(TIMERA0_BASE, 0x100);
    digitalWrite(BIN1, 0);
    digitalWrite(BIN2, 1);
  }
}

void setSpeed(uint16_t speedLeft, uint16_t speedRight)
{
  TimerMatchSet(TIMERA0_BASE, TIMER_A, speedLeft);
  TimerMatchSet(TIMERA0_BASE, TIMER_B, speedRight);
}

void setup()
{
  motorSetup();
  
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);
  
  accelerometer.begin();
  uint8_t chipID = accelerometer.chipID();
}

void loop()
{
  static int8_t error;
  /*static int8_t speed;
  static int8_t integral = 0;
  static int8_t derivate = 0;
  
  // Read accelerometer data
  error = accelerometer.readXData();
  
  // Do control stuff
  error = error - UPRIGHT_VALUE_ACCELEROMETER;
  integral = integral + error;
  derivate = derivate - error;
  
  speed = error * KP + integral * KI + derivate * KD;
  
  derivate = error;
  
  // Apply motor speeds
  
  setSpeed(speed, speed);

  delay(10);*/
  for(uint16_t i = 0; i < 4000; i += 100)
  {
    error = accelerometer.readXData();
    Serial.println(i);
    Serial.println(error);
    setSpeed(error * 10, error * 10);
    delay(500);
  }
  //digitalWrite(LED, !digitalRead(LED));
}


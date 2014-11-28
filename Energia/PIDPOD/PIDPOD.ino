/*
 Name:        PIDPOD
 Date:        2014-11-28
 Version:     1.0
 Description: Segway-type self-balancing robot sketch based on the CC3200 development board.
*/

// Includes for PWM signal generation
#include "Energia.h"
#include <driverlib/prcm.h>
#include <driverlib/rom_map.h>
#include <driverlib/pin.h>
#include <driverlib/timer.h>
#include <driverlib/adc.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gprcm.h>
#include <inc/hw_adc.h>

// Includes for BMA222 accelerometer communication
#include <Wire.h>
#include <BMA222.h>

#define UPRIGHT_VALUE_ACCELEROMETER 0
#define KP 1.
#define KI 0.
#define KD 0.

BMA222 accelerometer;

void motorSetup()
{
  /*uint32_t analog_res = 1023 * 1000;
  
  uint32_t load = (F_CPU / 20000) * 1000;
  uint32_t match = load / (1023 / 255);

  match = match;
  load = load / 1000;
  
  uint16_t prescaler = load >> 16;
  uint16_t prescaler_match = match >> 16;
  
  MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0 + (timer/2), PRCM_RUN_MODE_CLK);

  uint32_t base = TIMERA0_BASE + ((timer/2) << 12);
  
  MAP_TimerConfigure(base, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
  uint16_t timerab = timer % 2 ? TIMER_B : TIMER_A;
  
  MAP_TimerPrescaleSet(base, timerab, prescaler);
  MAP_TimerPrescaleMatchSet(base, timerab, prescaler_match);
  MAP_TimerControlLevel(base, timerab, 1);
  MAP_TimerLoadSet(base, timerab, load);
  MAP_TimerMatchSet(base, timerab, match);
  MAP_TimerEnable(base, timerab);*/
}

void setSpeed(int8_t speedLeft, int8_t speedRight)
{
  
}

void setup()
{
  motorSetup();
  
  Serial.begin(115200);
  
  accelerometer.begin();
  uint8_t chipID = accelerometer.chipID();
  Serial.print("chipID: ");
  Serial.println(chipID);
}

void loop()
{
  static int8_t error;
  static int8_t speed;
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

  delay(10);
}


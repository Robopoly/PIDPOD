/*
 * Name:        PIDPOD
 * Date:        2014-11-28
 * Version:     1.0
 * Description: Segway-type self-balancing robot sketch based on the CC3200 development board.
 */

//#define ENABLE_MOTORS

#include <Wire.h>
#include <MPU9150.h>
#include <Motors.h>

// sample direction value every 30ms
#define SAMPLE_TIME 30
// update upright position every 500ms
#define UPDATE_TIME 500
// upright position offset bias
#define UPRIGHT_OFFSET 1

float integral = 0;
float derivate = 0;

float acc_reading = 0;
float gyro_angle = 0;
float gyro_offset = 0;
float angle = 0;
float gyro_integration = 0;
float angle_offset = 0;
float angle_old = 0;
float angle_real = 0;
float distance = 0;
float upright_value_accelerometer = 0;

int16_t speed = 0;

// Pin definitions
#define SWAG_LED 5
#define DIP4 15
#define LED RED_LED

void setup()
{
  #ifdef ENABLE_MOTORS
  motorSetup();
  #endif
  
  // initialize serial communication
  Serial.begin(115200);
  
  pinMode(DIP4, INPUT);
  pinMode(SWAG_LED, OUTPUT);
  pinMode(LED, OUTPUT);
  
  // Initialize the 'Wire' class for the I2C-bus needed for IMU
  Wire.begin();
  
  delay(100);
  
  MPU9150_init();
  
  delay(100);
  
  // Find accelerometer and gyroscope offset
  float accelerometer = 0;
  gyro_offset = 0;
  uint8_t i;
  for(i = 0; i < 16; i++)
  {
    accelerometer += MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H);
    gyro_offset += MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H);
  }
  // average
  upright_value_accelerometer = accelerometer/16;
  gyro_offset /= 16;
  
  // Setup done
  digitalWrite(LED, HIGH);
  digitalWrite(SWAG_LED, HIGH);
  
  Serial.println(upright_value_accelerometer);
}

void loop()
{
  static uint32_t timeSample = millis() + SAMPLE_TIME;
  static uint32_t timeUpdate = millis() + UPDATE_TIME;
  
  static uint32_t lastTime = micros();
  static uint32_t dt = micros();
  
  acc_reading = -(MPU9150_readSensor(MPU9150_ACCEL_ZOUT_L, MPU9150_ACCEL_ZOUT_H) - upright_value_accelerometer) / 1000;
  gyro_angle = (MPU9150_readSensor(MPU9150_GYRO_XOUT_L, MPU9150_GYRO_XOUT_H) - gyro_offset) / 200;
  dt = micros() - lastTime;
  
  angle = (angle + gyro_angle * dt / 1000000) * 0.98 + (acc_reading * 0.02);
  
  integral = integral + angle * .1;
  // ARW
  if(integral > 10)
  {
    integral = 10;
  }
  else if(integral < -10)
  {
    integral = -10;
  }
  
  lastTime = micros();
  
  speed = angle * 10 + gyro_angle * 0.6 + integral * 11;
  
  // Apply motor speeds
  #ifdef ENABLE_MOTORS
  if(digitalRead(DIP4))
  {
    setSpeed(speed, speed);
  }
  else
  {
    setSpeed(0, 0);
  }
  #endif
  
  static uint8_t mem = 0;
  static int16_t memory[8] = {0};
  if(millis() > timeSample)
  {
    timeSample = millis() + SAMPLE_TIME;
    memory[mem] = speed;
    
    Serial.print("Mem ");
    Serial.print(mem);
    Serial.print(" : ");
    Serial.println(memory[mem]);
    
    mem++;
    if(mem == 8)
    {
      mem = 0;
    }
  }
  
  if(millis() > timeUpdate)
  {
    timeUpdate = millis() + UPDATE_TIME;
    int8_t bias = 0;
    for(uint8_t k = 0; k < 8; k++)
    {
      if(memory[k] > 0)
      {
        bias++;
      }
      else if(memory[k] < 0)
      {
        bias--;
      }
    }
    
    if(bias > 0)
    {
      upright_value_accelerometer -= UPRIGHT_OFFSET;
    }
    else
    {
      upright_value_accelerometer += UPRIGHT_OFFSET;
    }
    Serial.print("Bias: ");
    Serial.print(bias);
    Serial.print(" -> ");
    Serial.println(upright_value_accelerometer);
  }
  
  //delay(10);
}


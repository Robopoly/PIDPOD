/*
 * Name:        PIDPOD
 * Date:        2014-11-28
 * Version:     1.0
 * Description: Segway-type self-balancing robot sketch based on the CC3200 development board.
 */

//#define ENABLE_MOTORS
 

#include <Wire.h>

// Register names according to the datasheet.
// According to the InvenSense document
// "MPU-9150 Register Map and Descriptions Revision 4.0",

#define MPU9150_SELF_TEST_X        0x0D   // R/W
#define MPU9150_SELF_TEST_Y        0x0E   // R/W
#define MPU9150_SELF_TEST_X        0x0F   // R/W
#define MPU9150_SELF_TEST_A        0x10   // R/W
#define MPU9150_SMPLRT_DIV         0x19   // R/W
#define MPU9150_CONFIG             0x1A   // R/W
#define MPU9150_GYRO_CONFIG        0x1B   // R/W
#define MPU9150_ACCEL_CONFIG       0x1C   // R/W
#define MPU9150_FF_THR             0x1D   // R/W
#define MPU9150_FF_DUR             0x1E   // R/W
#define MPU9150_MOT_THR            0x1F   // R/W
#define MPU9150_MOT_DUR            0x20   // R/W
#define MPU9150_ZRMOT_THR          0x21   // R/W
#define MPU9150_ZRMOT_DUR          0x22   // R/W
#define MPU9150_FIFO_EN            0x23   // R/W
#define MPU9150_I2C_MST_CTRL       0x24   // R/W
#define MPU9150_I2C_SLV0_ADDR      0x25   // R/W
#define MPU9150_I2C_SLV0_REG       0x26   // R/W
#define MPU9150_I2C_SLV0_CTRL      0x27   // R/W
#define MPU9150_I2C_SLV1_ADDR      0x28   // R/W
#define MPU9150_I2C_SLV1_REG       0x29   // R/W
#define MPU9150_I2C_SLV1_CTRL      0x2A   // R/W
#define MPU9150_I2C_SLV2_ADDR      0x2B   // R/W
#define MPU9150_I2C_SLV2_REG       0x2C   // R/W
#define MPU9150_I2C_SLV2_CTRL      0x2D   // R/W
#define MPU9150_I2C_SLV3_ADDR      0x2E   // R/W
#define MPU9150_I2C_SLV3_REG       0x2F   // R/W
#define MPU9150_I2C_SLV3_CTRL      0x30   // R/W
#define MPU9150_I2C_SLV4_ADDR      0x31   // R/W
#define MPU9150_I2C_SLV4_REG       0x32   // R/W
#define MPU9150_I2C_SLV4_DO        0x33   // R/W
#define MPU9150_I2C_SLV4_CTRL      0x34   // R/W
#define MPU9150_I2C_SLV4_DI        0x35   // R  
#define MPU9150_I2C_MST_STATUS     0x36   // R
#define MPU9150_INT_PIN_CFG        0x37   // R/W
#define MPU9150_INT_ENABLE         0x38   // R/W
#define MPU9150_INT_STATUS         0x3A   // R  
#define MPU9150_ACCEL_XOUT_H       0x3B   // R  
#define MPU9150_ACCEL_XOUT_L       0x3C   // R  
#define MPU9150_ACCEL_YOUT_H       0x3D   // R  
#define MPU9150_ACCEL_YOUT_L       0x3E   // R  
#define MPU9150_ACCEL_ZOUT_H       0x3F   // R  
#define MPU9150_ACCEL_ZOUT_L       0x40   // R  
#define MPU9150_TEMP_OUT_H         0x41   // R  
#define MPU9150_TEMP_OUT_L         0x42   // R  
#define MPU9150_GYRO_XOUT_H        0x43   // R  
#define MPU9150_GYRO_XOUT_L        0x44   // R  
#define MPU9150_GYRO_YOUT_H        0x45   // R  
#define MPU9150_GYRO_YOUT_L        0x46   // R  
#define MPU9150_GYRO_ZOUT_H        0x47   // R  
#define MPU9150_GYRO_ZOUT_L        0x48   // R  
#define MPU9150_EXT_SENS_DATA_00   0x49   // R  
#define MPU9150_EXT_SENS_DATA_01   0x4A   // R  
#define MPU9150_EXT_SENS_DATA_02   0x4B   // R  
#define MPU9150_EXT_SENS_DATA_03   0x4C   // R  
#define MPU9150_EXT_SENS_DATA_04   0x4D   // R  
#define MPU9150_EXT_SENS_DATA_05   0x4E   // R  
#define MPU9150_EXT_SENS_DATA_06   0x4F   // R  
#define MPU9150_EXT_SENS_DATA_07   0x50   // R  
#define MPU9150_EXT_SENS_DATA_08   0x51   // R  
#define MPU9150_EXT_SENS_DATA_09   0x52   // R  
#define MPU9150_EXT_SENS_DATA_10   0x53   // R  
#define MPU9150_EXT_SENS_DATA_11   0x54   // R  
#define MPU9150_EXT_SENS_DATA_12   0x55   // R  
#define MPU9150_EXT_SENS_DATA_13   0x56   // R  
#define MPU9150_EXT_SENS_DATA_14   0x57   // R  
#define MPU9150_EXT_SENS_DATA_15   0x58   // R  
#define MPU9150_EXT_SENS_DATA_16   0x59   // R  
#define MPU9150_EXT_SENS_DATA_17   0x5A   // R  
#define MPU9150_EXT_SENS_DATA_18   0x5B   // R  
#define MPU9150_EXT_SENS_DATA_19   0x5C   // R  
#define MPU9150_EXT_SENS_DATA_20   0x5D   // R  
#define MPU9150_EXT_SENS_DATA_21   0x5E   // R  
#define MPU9150_EXT_SENS_DATA_22   0x5F   // R  
#define MPU9150_EXT_SENS_DATA_23   0x60   // R  
#define MPU9150_MOT_DETECT_STATUS  0x61   // R  
#define MPU9150_I2C_SLV0_DO        0x63   // R/W
#define MPU9150_I2C_SLV1_DO        0x64   // R/W
#define MPU9150_I2C_SLV2_DO        0x65   // R/W
#define MPU9150_I2C_SLV3_DO        0x66   // R/W
#define MPU9150_I2C_MST_DELAY_CTRL 0x67   // R/W
#define MPU9150_SIGNAL_PATH_RESET  0x68   // R/W
#define MPU9150_MOT_DETECT_CTRL    0x69   // R/W
#define MPU9150_USER_CTRL          0x6A   // R/W
#define MPU9150_PWR_MGMT_1         0x6B   // R/W
#define MPU9150_PWR_MGMT_2         0x6C   // R/W
#define MPU9150_FIFO_COUNTH        0x72   // R/W
#define MPU9150_FIFO_COUNTL        0x73   // R/W
#define MPU9150_FIFO_R_W           0x74   // R/W
#define MPU9150_WHO_AM_I           0x75   // R

//MPU9150 Compass
#define MPU9150_CMPS_XOUT_L        0x4A   // R
#define MPU9150_CMPS_XOUT_H        0x4B   // R
#define MPU9150_CMPS_YOUT_L        0x4C   // R
#define MPU9150_CMPS_YOUT_H        0x4D   // R
#define MPU9150_CMPS_ZOUT_L        0x4E   // R
#define MPU9150_CMPS_ZOUT_H        0x4F   // R

// I2C address 0x69 could be 0x68 depends on your wiring. 
int MPU9150_I2C_ADDRESS = 0x68;

//Variables where our values can be stored
int cmps[3];
int accl[3];
int gyro[3];
int temp;

#include <inc/hw_types.h>
#include <inc/hw_timer.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
//#include <driverlib/adc.h>

#define MOTOR_PRESCALER 8000
#define MOTOR_OFFSET 400
#define AVG  50

int8_t speedLeft = 0;
int8_t speedRight = 0;

int16_t speed = 0;
int16_t error = 0;
int16_t memory[AVG];
int16_t error_avg = 0;
int16_t integral = 0;
int16_t derivate = 0;

uint8_t i,j;

// Includes for BMA222 accelerometer communication
//#include <Wire.h>
//#include <BMA222.h>

// Wi-Fi server stuff
/*#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>*/

// server instance
//WiFiServer server(80);

// PID coefficients
#define UPRIGHT_VALUE_ACCELEROMETER 22
#define KP 6.
#define KI 0.
#define KD 0.

//BMA222 accelerometer;

// Pin definitions
#define LED RED_LED
#define AIN1x	30
#define AIN2x	28
#define BIN1x	8
#define BIN2x	7
#define AISEN	23
#define BISEN	2

uint8_t bitA1, bitA2, bitB1, bitB2;
uint8_t portA1, portA2, portB1, portB2;
uint32_t baseA1, baseA2, baseB1, baseB2;

// your network name also called SSID
char ssid[] = "networkname";
// your network password
char password[] = "password";
// your network key Index number (needed only for WEP)
int keyIndex = 0;

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
  MAP_TimerPrescaleSet(TIMERA0_BASE,TIMER_A,0);
  MAP_TimerPrescaleSet(TIMERA0_BASE,TIMER_B,0);

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

void setup()
{
  #ifdef ENABLE_MOTORS
  motorSetup();
  #endif
  
  // initialize serial communication
  Serial.begin(115200);
  
  //pinMode(SDA, INPUT_PULLUP);
  //pinMode(SCL, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();

  // Clear the 'sleep' bit to start the sensor.
  MPU9150_writeSensor(MPU9150_PWR_MGMT_1, 0);

  MPU9150_setupCompass();
  
  // Wi-Fi stuff

  // attempt to connect to Wifi network:
  /*Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while(WiFi.localIP() == INADDR_NONE)
  {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  // you're connected now, so print out the status  
  printWifiStatus();
  
  Serial.println("Starting webserver on port 80");
  server.begin();                           // start the web server on port 80
  Serial.println("Webserver started!");*/
}

void loop()
{
  Serial.println(MPU9150_readSensor(MPU9150_GYRO_XOUT_L,MPU9150_GYRO_XOUT_H));
  // Read accelerometer data
  //error = accelerometer.readZData();
  
  delay(100);
  
  /*Serial.println(error_avg);
  delay(100);*/
  
  // Do control stuff
  /*error = error - UPRIGHT_VALUE_ACCELEROMETER;
  
  memory[i] = error;
  i++;
  
  error_avg = 0;
  
  for(j=0;j<AVG;j++)
    error_avg += memory[j];
    
  error_avg /= AVG;
  
  if(i > AVG)
    i = 0;
  
  
  
  
  //integral = integral + error;
  derivate = error_avg - derivate;
  
  speed = error_avg * KP + derivate * KD;
  //speed = error * KP + integral * KI + derivate * KD;
  
  derivate = error;
  
  // Apply motor speeds
  
  setSpeed(speed, speed);
  //setSpeed(0, 0);
  delayMicroseconds(200);*/
  
  // min = 400
  // max = 3600
  /*for(int8_t i = -50; i < 50; i++)
  {
    //error = accelerometer.readXData();
    setSpeed(i, i);
    delay(100);
  }*/
  
  //webServer();
}

/*void webServer()
{
  // listen for incoming clients
  WiFiClient client = server.available();
  if(client)
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while(client.connected())
    {
      if(client.available())
      {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if(c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          
          // output the value of each analog input pin
          for(int analogChannel = 0; analogChannel < 6; analogChannel++)
          {
            int sensorReading = analogChannel; //analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if(c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if(c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("Network Name: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}*/

// MPU-9150 Accelerometer + Gyro + Compass + Temperature
// -----------------------------
//
// By arduino.cc user "frtrobotik" (Tobias Hübner)
//
//
// July 2013
//      first version
//
// Open Source / Public Domain
//
// Using Arduino 1.0.1
// It will not work with an older version,
// since Wire.endTransmission() uses a parameter
// to hold or release the I2C bus.
//
// Documentation:
// - The InvenSense documents:
//   - "MPU-9150 Product Specification Revision 4.0",
//     PS-MPU-9150A.pdf
//   - "MPU-9150 Register Map and Descriptions Revision 4.0",
//     RM-MPU-9150A-00.pdf
//   - "MPU-9150 9-Axis Evaluation Board User Guide"
//     AN-MPU-9150EVB-00.pdf
//
// The accuracy is 16-bits.
//
// Some parts are copied by the MPU-6050 Playground page.
// playground.arduino.cc/Main/MPU-6050
// There are more Registervalues. Here are only the most
// nessecary ones to get started with this sensor.

//http://pansenti.wordpress.com/2013/03/26/pansentis-invensense-mpu-9150-software-for-arduino-is-now-on-github/
//Thank you to pansenti for setup code.
//I will documented this one later.
void MPU9150_setupCompass(){
  MPU9150_I2C_ADDRESS = 0x0C;      //change Adress to Compass

  MPU9150_writeSensor(0x0A, 0x00); //PowerDownMode
  MPU9150_writeSensor(0x0A, 0x0F); //SelfTest
  MPU9150_writeSensor(0x0A, 0x00); //PowerDownMode

  MPU9150_I2C_ADDRESS = 0x68;      //change Adress to MPU

  MPU9150_writeSensor(0x24, 0x40); //Wait for Data at Slave0
  MPU9150_writeSensor(0x25, 0x8C); //Set i2c address at slave0 at 0x0C
  MPU9150_writeSensor(0x26, 0x02); //Set where reading at slave 0 starts
  MPU9150_writeSensor(0x27, 0x88); //set offset at start reading and enable
  MPU9150_writeSensor(0x28, 0x0C); //set i2c address at slv1 at 0x0C
  MPU9150_writeSensor(0x29, 0x0A); //Set where reading at slave 1 starts
  MPU9150_writeSensor(0x2A, 0x81); //Enable at set length to 1
  MPU9150_writeSensor(0x64, 0x01); //overvride register
  MPU9150_writeSensor(0x67, 0x03); //set delay rate
  MPU9150_writeSensor(0x01, 0x80);

  MPU9150_writeSensor(0x34, 0x04); //set i2c slv4 delay
  MPU9150_writeSensor(0x64, 0x00); //override register
  MPU9150_writeSensor(0x6A, 0x00); //clear usr setting
  MPU9150_writeSensor(0x64, 0x01); //override register
  MPU9150_writeSensor(0x6A, 0x20); //enable master i2c mode
  MPU9150_writeSensor(0x34, 0x13); //disable slv4
}

////////////////////////////////////////////////////////////
///////// I2C functions to get easier all values ///////////
////////////////////////////////////////////////////////////

int MPU9150_readSensor(int addrL, int addrH){
  Wire.beginTransmission(MPU9150_I2C_ADDRESS);
  Wire.write(addrL);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU9150_I2C_ADDRESS, 1, true);
  byte L = Wire.read();

  Wire.beginTransmission(MPU9150_I2C_ADDRESS);
  Wire.write(addrH);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU9150_I2C_ADDRESS, 1, true);
  byte H = Wire.read();

  return (int16_t)((H<<8)+L);
}

int MPU9150_readSensor(int addr){
  Wire.beginTransmission(MPU9150_I2C_ADDRESS);
  Wire.write(addr);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU9150_I2C_ADDRESS, 1, true);
  return Wire.read();
}

int MPU9150_writeSensor(int addr,int data){
  Wire.beginTransmission(MPU9150_I2C_ADDRESS);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission(true);

  return 1;
}


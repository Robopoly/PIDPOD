/* All GPIO, exept the ones from the motors, are setted, written aand read from this file */

#ifndef _PIDPOD_GPIO_H_
#define _PIDPOD_GPIO_H_

void InitGPIO(void);

uint8_t readDIP1(void);
uint8_t readDIP2(void);
uint8_t readDIP3(void);
uint8_t readDIP4(void);
uint8_t readODO1(void);
uint8_t readODO2(void);

void setRLED(void);
void clearRLED(void);
void setBLED(void);
void clearBLED(void);

uint16_t readAISEN(void);
uint16_t readBISEN(void);

#endif

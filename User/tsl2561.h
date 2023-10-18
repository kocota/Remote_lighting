/*
 * tsl2561.h
 *
 *  Created on: 15 ��. 2018 �.
 *      Author: Andriy
 */

#ifndef TSL2561_H_
#define TSL2561_H_

#include <stdbool.h>
#include <math.h>
//#include "stm32f1xx.h"
#include "stm32f4xx.h"

// #define TSL2561_ADDR_0 0x29 << 1	// address with '0' shorted on board
#define TSL2561_ADDR   0x39 << 1 	// default address
// #define TSL2561_ADDR_1 0x49 << 1	// address with '1' shorted on board

// TSL2561 registers
#define TSL2561_CMD           0x80
#define TSL2561_CMD_CLEAR     0xC0
#define	TSL2561_REG_CONTROL   0x00
#define	TSL2561_REG_TIMING    0x01
#define	TSL2561_REG_THRESH_L  0x02
#define	TSL2561_REG_THRESH_H  0x04
#define	TSL2561_REG_INTCTL    0x06
#define	TSL2561_REG_ID        0x0A
#define	TSL2561_REG_DATA_0    0x0C
#define	TSL2561_REG_DATA_1    0x0E

extern I2C_HandleTypeDef hi2c1;

bool TSL2561_setPowerUp(void);
bool TSL2561_setPowerDown(void);
bool TSL2561_setTiming(bool gain, uint8_t time);
bool TSL2561_setTiming_ms(bool gain, uint8_t time, unsigned int *ms);
bool TSL2561_manualStart(void);
bool TSL2561_manualStop(void);
bool TSL2561_getData(unsigned int *data0, unsigned int *data1);
bool TSL2561_getLux(unsigned char gain, unsigned int ms, unsigned int CH0, unsigned int CH1, double *lux);
bool TSL2561_setInterruptControl(unsigned char control, unsigned char persist);
bool TSL2561_setInterruptThreshold(unsigned int low, unsigned int high);
bool TSL2561_getID(unsigned char *ID);
bool TSL2561_WriteByte(uint8_t regAdr, uint8_t value);
bool TSL2561_ReadByte(uint8_t regAdr, uint8_t *value);
bool TSL2561_writeUInt(unsigned char address, unsigned int value);
bool TSL2561_readUInt(uint8_t address, unsigned int *value);

#endif /* TSL2561_H_ */

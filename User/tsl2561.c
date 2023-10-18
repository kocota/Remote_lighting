/*
 * tsl2561.c
 *
 *  Created on: 15 ��. 2018 �.
 *      Author: Andriy
 */
#include "tsl2561.h"

bool TSL2561_setPowerUp(void)
{
	// Write 0x03 to command byte (power on)
	return(TSL2561_WriteByte(TSL2561_REG_CONTROL,0x03));
}

bool TSL2561_setPowerDown(void)
{
	// Clear command byte (power off)
	return(TSL2561_WriteByte(TSL2561_REG_CONTROL,0x00));
}

bool TSL2561_setTiming(bool gain, uint8_t time)
	// If gain = false (0), device is set to low gain (1X)
	// If gain = high (1), device is set to high gain (16X)
	// If time = 0, integration will be 13.7ms
	// If time = 1, integration will be 101ms
	// If time = 2, integration will be 402ms
	// If time = 3, use manual start / stop
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint8_t timing;

	// Get timing byte
	if (TSL2561_ReadByte(TSL2561_REG_TIMING, &timing))
	{
		// Set gain (0 or 1)
		if (gain)
			timing |= 0x10;
		else
			timing &= ~0x10;

		// Set integration time (0 to 3)
		timing &= ~0x03;
		timing |= (time & 0x03);

		// Write modified timing byte back to device
		if (TSL2561_WriteByte(TSL2561_REG_TIMING, timing))
			return true;
	}
	return false;
}

bool TSL2561_setTiming_ms(bool gain, uint8_t time, unsigned int *ms)
	// If gain = false (0), device is set to low gain (1X)
	// If gain = high (1), device is set to high gain (16X)
	// If time = 0, integration will be 13.7ms
	// If time = 1, integration will be 101ms
	// If time = 2, integration will be 402ms
	// If time = 3, use manual start / stop (ms = 0)
	// ms will be set to integration time
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Calculate ms for user
	switch (time)
	{
		case 0:  *ms = 14; break;
		case 1:  *ms = 101; break;
		case 2:  *ms = 402; break;
		default: *ms = 0;
	}
	// Set integration using base function
	return(TSL2561_setTiming(gain, time));
}

bool TSL2561_manualStart(void)
	// Starts a manual integration period
	// After running this command, you must manually stop integration with manualStop()
	// Internally sets integration time to 3 for manual integration (gain is unchanged)
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint8_t timing;

	// Get timing byte
	if (TSL2561_ReadByte(TSL2561_REG_TIMING, &timing))
	{
		// Set integration time to 3 (manual integration)
		timing |= 0x03;

		if (TSL2561_WriteByte(TSL2561_REG_TIMING, timing))
		{
			// Begin manual integration
			timing |= 0x08;

			// Write modified timing byte back to device
			if (TSL2561_WriteByte(TSL2561_REG_TIMING, timing))
				return true;
		}
	}
	return false;
}

bool TSL2561_manualStop(void)
	// Stops a manual integration period
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint8_t timing;

	// Get timing byte
	if (TSL2561_ReadByte(TSL2561_REG_TIMING, &timing))
	{
		// Stop manual integration
		timing &= ~0x08;

		// Write modified timing byte back to device
		if (TSL2561_WriteByte(TSL2561_REG_TIMING, timing))
			return true;
	}
	return false;
}

bool TSL2561_getData(unsigned int *data0, unsigned int *data1)
	// Retrieve raw integration results
	// data0 and data1 will be set to integration results
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Get data0 and data1 out of result registers
	return TSL2561_readUInt(TSL2561_REG_DATA_0, data0) && TSL2561_readUInt(TSL2561_REG_DATA_1, data1);
}

bool TSL2561_getLux(unsigned char gain, unsigned int ms, unsigned int CH0, unsigned int CH1, double *lux)
	// Convert raw data to lux
	// gain: 0 (1X) or 1 (16X), see setTiming()
	// ms: integration time in ms, from setTiming() or from manual integration
	// CH0, CH1: results from getData()
	// lux will be set to resulting lux calculation
	// returns true (1) if calculation was successful
	// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{
	double ratio, d0, d1;

	// Determine if either sensor saturated (0xFFFF)
	// If so, abandon ship (calculation will not be accurate)
	if ((CH0 == 0xFFFF) || (CH1 == 0xFFFF))
	{
		*lux = 0.0;
		return false;
	}

	// Convert from unsigned integer to floating point
	d0 = CH0; d1 = CH1;

	// We will need the ratio for subsequent calculations
	ratio = d1 / d0;

	// Normalize for integration time
	d0 *= (402.0/ms);
	d1 *= (402.0/ms);

	// Normalize for gain
	if (!gain)
	{
		d0 *= 16;
		d1 *= 16;
	}

	// Determine lux per datasheet equations:

	if (ratio < 0.5)
	{
		*lux = 0.0304 * d0 - 0.062 * d0 * pow(ratio,1.4);
		return true;
	}

	if (ratio < 0.61)
	{
		*lux = 0.0224 * d0 - 0.031 * d1;
		return true;
	}

	if (ratio < 0.80)
	{
		*lux = 0.0128 * d0 - 0.0153 * d1;
		return true;
	}

	if (ratio < 1.30)
	{
		*lux = 0.00146 * d0 - 0.00112 * d1;
		return true;
	}

	// if (ratio > 1.30)
	*lux = 0.0;
	return true;
}

bool TSL2561_setInterruptControl(unsigned char control, unsigned char persist)
	// Sets up interrupt operations
	// If control = 0, interrupt output disabled
	// If control = 1, use level interrupt, see setInterruptThreshold()
	// If persist = 0, every integration cycle generates an interrupt
	// If persist = 1, any value outside of threshold generates an interrupt
	// If persist = 2 to 15, value must be outside of threshold for 2 to 15 integration cycles
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Place control and persist bits into proper location in interrupt control register
	if (TSL2561_WriteByte(TSL2561_REG_INTCTL,((control | 0B00000011) << 4) & (persist | 0B00001111)))
	{
		return true;
	}

	return false;
}

bool TSL2561_setInterruptThreshold(unsigned int low, unsigned int high)
	// Set interrupt thresholds (channel 0 only)
	// low, high: 16-bit threshold values
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Write low and high threshold values
	if (TSL2561_writeUInt(TSL2561_REG_THRESH_L, low) && TSL2561_writeUInt(TSL2561_REG_THRESH_H, high))
	{
		return true;
	}

	return false;
}

bool TSL2561_clearInterrupt(void)
	// Clears an active interrupt
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Set up command byte for interrupt clear
	uint8_t data = TSL2561_CMD_CLEAR;
	if(HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) TSL2561_ADDR, &data, 1, 1000) == HAL_OK)
	{
		return true;
	}

	return false;
}

bool TSL2561_getID(unsigned char *ID)
	// Retrieves part and revision code from TSL2561
	// Sets ID to part ID (see datasheet)
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Get ID byte from ID register
	if (TSL2561_ReadByte(TSL2561_REG_ID, ID))
	{
		return true;
	}

	return false;
}

bool TSL2561_WriteByte(uint8_t regAdr, uint8_t value)
{
	uint8_t data[2] = {0, 0};

	data[0] = ((regAdr & 0x0F) | TSL2561_CMD);
	data[1] = value;

	if(HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) TSL2561_ADDR, data, 2, 1000) == HAL_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TSL2561_ReadByte(uint8_t regAdr, uint8_t *value)
{
	uint8_t data = ((regAdr & 0x0F) | TSL2561_CMD);

	if(HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) TSL2561_ADDR, &data, 1, 1000) == HAL_OK)
	{
		if(HAL_I2C_Master_Receive(&hi2c1, (uint16_t) TSL2561_ADDR, value, 1, 1000) == HAL_OK)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool TSL2561_readUInt(uint8_t address, unsigned int *value)
	// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
	// Address: TSL2561 address (0 to 15), low byte first
	// Value will be set to stored unsigned integer
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	char high, low;

	// Set up command byte for read
	uint8_t data = ((address & 0x0F) | TSL2561_CMD);
	uint8_t tmpValue[2] = {0, 0};

	if(HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) TSL2561_ADDR, &data, 1, 1000) == HAL_OK)
	{
		if(HAL_I2C_Master_Receive(&hi2c1, (uint16_t) TSL2561_ADDR, tmpValue, 2, 1000) == HAL_OK)
		{
			low = tmpValue[0];
			high = tmpValue[1];
			// Combine bytes into unsigned int
			*value = high << 8 |  low;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool TSL2561_writeUInt(unsigned char address, unsigned int value)
	// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
	// Address: TSL2561 address (0 to 15), low byte first
	// Value: unsigned int to write to address
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	uint8_t low = value & 0xFF;
	uint8_t high = value >> 8;
	// Split int into lower and upper bytes, write each byte
	if (TSL2561_WriteByte(address, low)	&& TSL2561_WriteByte(address + 1, high))
	{
		return true;
	}

	return false;
}

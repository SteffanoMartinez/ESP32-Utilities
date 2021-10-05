/*************************************************************************
Title:    ARD-LTC2499 Driver Library
Authors:  Nathan D. Holmes <maverick@drgw.net>
File:     $Id: $
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2017 Nathan D. Holmes & Michael D. Petersen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "Arduino.h"

#include "Ard1863.h"

#define ARD186x_SPI_CLOCK_FREQ 40000000

void ARD1867::ltc186xSleep()
{
	current186xConfig |= _BV(LTC186X_CONFIG_SLP);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.beginTransaction(SPISettings(ARD186x_SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE0));

	digitalWrite(this->cs_pin, LOW);
	spi_class.transfer(current186xConfig);
	spi_class.transfer(0);
	digitalWrite(this->cs_pin, HIGH);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.endTransaction();
}

void ARD1867::ltc186xWake()
{
	uint8_t wasSleeping = current186xConfig & _BV(LTC186X_CONFIG_SLP);
	current186xConfig &= ~_BV(LTC186X_CONFIG_SLP);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.beginTransaction(SPISettings(ARD186x_SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE0));

	digitalWrite(this->cs_pin, LOW);
	spi_class.transfer(current186xConfig);
	spi_class.transfer(0);
	digitalWrite(this->cs_pin, HIGH);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.endTransaction();

	// Wake-up time if we were really sleeping
	if (wasSleeping)
		delay(70);
}

unsigned int ARD1867::ltc186xRead()
{
	uint16_t retval = 0;
	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.beginTransaction(SPISettings(ARD186x_SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE0));

	digitalWrite(this->cs_pin, LOW);
	if (DEVICE_LTC1863 == ltc186xDeviceType)
	{
		retval = spi_class.transfer(current186xConfig);
		retval <<= 4;
		retval |= 0x0F & (spi_class.transfer(0) >> 4);
	}
	else if (DEVICE_LTC1867 == ltc186xDeviceType)
	{
		retval = spi_class.transfer(current186xConfig);
		retval <<= 8;
		retval |= 0xFF & spi_class.transfer(0);
	}

	digitalWrite(this->cs_pin, HIGH);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.endTransaction();

	return (retval);
}

int ARD1867::ltc186xReadBipolar()
{
	int retval = (int)ltc186xRead();

	if (DEVICE_LTC1863 == ltc186xDeviceType && (retval & 0x0800))
		retval |= 0xF000;

	return (retval);
}

void ARD1867::ltc186xChangeChannel(byte nextChannel, byte unipolar = 1)
{
	internalChangeChannel(nextChannel, unipolar);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.beginTransaction(SPISettings(ARD186x_SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE0));

	digitalWrite(this->cs_pin, LOW);
	spi_class.transfer(current186xConfig);
	spi_class.transfer(0);
	digitalWrite(this->cs_pin, HIGH);

	if (!(this->flags & ARD186X_SKIP_SPI_TRANSACTION))
		spi_class.endTransaction();
}

byte ARD1867::internalChangeChannel(byte nextChannel, byte unipolar)
{
	switch (nextChannel)
	{
	case 0:
		current186xConfig = LTC186X_CHAN_SINGLE_0P;
		break;
	case 1:
		current186xConfig = LTC186X_CHAN_SINGLE_1P;
		break;
	case 2:
		current186xConfig = LTC186X_CHAN_SINGLE_2P;
		break;

	default:
		break;
	}
}

unsigned int ARD1867::ltc186xReadAndChangeChannel(byte nextChannel, byte unipolar = 1)
{
	internalChangeChannel(nextChannel, unipolar);
	return (ltc186xRead());
}

int ARD1867::ltc186xReadBipolarAndChangeChannel(byte nextChannel, byte unipolar = 0)
{
	internalChangeChannel(nextChannel, unipolar);
	return (ltc186xReadBipolar());
}

ARD1867::ARD1867()
{
	init_status = ARD186X_LTC186X_ERR | ARD186X_EEPROM_ERR;
	ltc186xDeviceType = 0;
	i2cAddr_eeprom = 0;
	current186xConfig = LTC186X_CHAN_DIFF_0P_1N;

#ifdef ARDUINO_SAM_DUE
	WireInterface = &Wire1;
#else
	WireInterface = &Wire;
#endif

	strcpy(eui48, "Unknown");
}

const char *ARD1867::eui48Get()
{
	return (eui48);
}

void ARD1867::setFastSPI(byte speedySPI)
{
	if (speedySPI)
		this->flags |= ARD186X_SKIP_SPI_TRANSACTION;
}

byte ARD1867::begin(SPIClass spi_in_class, byte deviceType, byte eepromAddress, int adc_cs_pin = 3)
{
	spi_class = spi_in_class;
	byte retval = 0;
	byte i;
	this->flags = 0;
	cs_pin = adc_cs_pin;

	pinMode(this->cs_pin, OUTPUT);
	digitalWrite(this->cs_pin, HIGH);

	init_status = ARD186X_SUCCESS;
	strcpy(eui48, "Unknown");

	if (DEVICE_LTC1863 == deviceType || DEVICE_LTC1867 == deviceType)
		ltc186xDeviceType = deviceType;
	else
		init_status |= ARD186X_LTC186X_ERR;

	current186xConfig = LTC186X_CHAN_SINGLE_0P;

	spi_class.beginTransaction(SPISettings(ARD186x_SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE0));
	digitalWrite(this->cs_pin, LOW);
	spi_class.transfer(current186xConfig);
	spi_class.transfer(0);
	spi_class.transfer(255);
	digitalWrite(this->cs_pin, HIGH);
	spi_class.endTransaction();

	this->WireInterface->begin();

	i2cAddr_eeprom = eepromAddress;
	if (ARD186X_EEP_DISABLE != i2cAddr_eeprom)
	{
		// ARD186x_EEP_DISABLE means that we're not going to use the EEPROM
		// This is useful on boards like the Leonardo where /CS (D3) steps on an I2C pin
		this->WireInterface->beginTransmission(i2cAddr_eeprom);
		this->WireInterface->write(ARD186X_EEPROM_ADDR_EUI48);
		retval = this->WireInterface->endTransmission(false);
		// Anything but zero means we couldn't initialize the LTC2499
		if (0 != retval)
		{
			// Make sure we send a stop bit
			this->WireInterface->endTransmission(true);
			i2cAddr_eeprom = 0;
			init_status |= ARD186X_EEPROM_ERR;
		}
		else
		{

			this->WireInterface->requestFrom(i2cAddr_eeprom, (uint8_t)6, (uint8_t) true);
			if (this->WireInterface->available() < 6)
			{
				i2cAddr_eeprom = 0;
				init_status |= ARD186X_EEPROM_ERR;
			}
			else
			{
				memset(eui48, 0, sizeof(eui48));
				for (i = 0; i < 12; i += 2)
					sprintf(&eui48[i], "%02X", this->WireInterface->read());
			}
		}
	}

	return (init_status);
}

byte ARD1867::eepromRead(int address, byte defaultOnError = 0)
{
	byte retval = 0;

	// EEPROM was disabled in initialization
	if (ARD186X_EEP_DISABLE == i2cAddr_eeprom)
		return (defaultOnError);

	// Our address range only goes to 0xFF
	if (address > 0xFF)
		return (defaultOnError);

	this->WireInterface->beginTransmission(i2cAddr_eeprom);
	this->WireInterface->write((uint8_t)address);
	retval = this->WireInterface->endTransmission(false);
	// Anything but zero means we couldn't initialize the EEPROM
	if (0 != retval)
	{
		// Make sure we send a stop bit
		this->WireInterface->endTransmission(true);
		return (0);
	}

	this->WireInterface->requestFrom((uint8_t)i2cAddr_eeprom, (uint8_t)1, (uint8_t) true);
	if (this->WireInterface->available() < 1)
		return (defaultOnError);
	return (this->WireInterface->read());
}

byte ARD1867::eepromWrite(int address, byte value, byte blocking = 1)
{
	uint8_t waitLoop = 10;
	byte retval = 0;

	// EEPROM was disabled in initialization
	if (ARD186X_EEP_DISABLE == i2cAddr_eeprom)
		return (ARD186X_EEPROM_ERR);

	// Our writable address range only goes to 0xFF
	if (address > 0x7F)
		return (ARD186X_EEPROM_ERR);

	this->WireInterface->beginTransmission(i2cAddr_eeprom);
	this->WireInterface->write(address);
	this->WireInterface->write(value);
	retval = this->WireInterface->endTransmission(true);
	// Anything but zero means we couldn't write to the EEPROM
	if (0 != retval)
	{
		return (ARD186X_EEPROM_ERR);
	}

	if (0 != blocking)
	{
		while (0 != waitLoop--)
		{
			this->WireInterface->beginTransmission(i2cAddr_eeprom);
			this->WireInterface->write(address);
			if (0 == this->WireInterface->endTransmission(true))
				return (0);
			delay(1);
		}
		return (ARD186X_EEPROM_ERR);
	}
	return (ARD186X_SUCCESS);
}

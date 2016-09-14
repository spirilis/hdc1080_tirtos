/**
 *
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h>
// HDC1080 driver
#include "hdc1080.h"


static I2C_Handle iH;
static HDC1080_Struct chipData;

Void HDC1080_init()
{
	chipData.temp = 0;
	chipData.humidity = 0;
	chipData.heaterOn = false;
}

Bool HDC1080_open(UInt i2cInstance)
{
	I2C_Params iP;
	I2C_Handle hand;

	I2C_Params_init(&iP);
	iP.bitRate = I2C_400kHz;
	iP.transferMode = I2C_MODE_BLOCKING;
	iP.transferCallbackFxn = NULL;

	hand = I2C_open(i2cInstance, &iP);
	if (hand == NULL) {
		System_printf("Error in I2C_open\n"); System_flush();
		return false;
	}

	return HDC1080_quickopen(hand);
}

Bool HDC1080_quickopen(I2C_Handle i2cbus)
{
	iH = i2cbus;

	I2C_Transaction txn;
	uint8_t rd[4], wr;

	// Set configuration
	uint16_t configurationValue = 0x1000;
	txn.readCount = 0;
	txn.writeBuf = rd;
	txn.writeCount = 3;
	rd[0] = 0x02;  // Register 0x02, CONFIGURATION
	rd[1] = configurationValue >> 8;
	rd[2] = configurationValue & 0xFF;
	I2C_transfer(iH, &txn);

	// Read Mfr, Device ID to validate whether the chip is really there
	txn.readBuf = rd;
	txn.readCount = 2;
	wr = 0xFE;  // Manufacturer ID
	txn.writeBuf = &wr;
	txn.writeCount = 1;
	txn.slaveAddress = HDC1080_SLAVE_ADDR;
	I2C_transfer(iH, &txn);
	wr = 0xFF;  // Device ID (Can only read 1 word at a time...)
	txn.readBuf = rd+2;
	I2C_transfer(iH, &txn);

	uint16_t d = (uint16_t)rd[0] << 8 | (uint16_t)rd[1];
	System_printf("rd[4]: %x %x %x %x\n", rd[0], rd[1], rd[2], rd[3]); System_flush();
	if (d != 0x5449) {  // 0x5449 = Texas Instruments
		return false;
	}
	d = (uint16_t)rd[2] << 8 | (uint16_t)rd[3];
	if (d != 0x1050) {  // 0x1050 = HDC1080
		return false;
	}

	return true;
}

Void HDC1080_close()
{
	if (iH) {
		I2C_close(iH);
	}
}

HDC1080_Handle HDC1080_read()
{
	I2C_Transaction txn;

	uint8_t rd[4], wr;
	wr = 0x00;  // Pointer to TEMPERATURE reading; write pointer to initiate conversion
	txn.slaveAddress = HDC1080_SLAVE_ADDR;
	txn.readCount = 0;
	txn.writeBuf = &wr;
	txn.writeCount = 1;

	I2C_transfer(iH, &txn);  // Bang!

	Task_sleep(15000 / Clock_tickPeriod);  // Sleep for 15ms until conversion is complete.

	txn.writeCount = 0;
	txn.readBuf = rd;
	txn.readCount = 4;
	I2C_transfer(iH, &txn);

	chipData.temp = (uint16_t)rd[0] << 8 | (uint16_t)rd[1];
	chipData.humidity = (uint16_t)rd[2] << 8 | (uint16_t)rd[3];

	return (HDC1080_Handle)&chipData;
}

Void HDC1080_setConfiguration(uint16_t cfg)
{
	I2C_Transaction txn;

	// Set configuration
	uint8_t wr[3];
	txn.readCount = 0;
	txn.writeBuf = wr;
	txn.writeCount = 3;
	wr[0] = 0x02;  // Register 0x02, CONFIGURATION
	wr[1] = cfg >> 8;
	wr[2] = cfg & 0xFF;
	I2C_transfer(iH, &txn);
}

Void HDC1080_heaterOn()
{
	HDC1080_setConfiguration(0x3000);
	chipData.heaterOn = true;
}

Void HDC1080_heaterOff()
{
	HDC1080_setConfiguration(0x1000);
	chipData.heaterOn = false;
}

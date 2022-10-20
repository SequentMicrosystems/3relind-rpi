/*
 * comm.c:
 *	Communication routines "platform specific" for Raspberry Pi
 *	
 *	Copyright (c) 2016-2020 Sequent Microsystem
 *	<http://www.sequentmicrosystem.com>
 ***********************************************************************
 *	Author: Alexandru Burcea
 ***********************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "comm.h"

#define I2C_SLAVE	0x0703
#define I2C_SMBUS	0x0720	/* SMBus-level access */

#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0

// SMBus transaction types

#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		/* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA    8

// SMBus messages

#define I2C_SMBUS_BLOCK_MAX	32	/* As specified in SMBus standard */
#define I2C_SMBUS_I2C_BLOCK_MAX	32	/* Not specified but we use same structure */


int i2cSetup(int addr)
{
	int file;
	char filename[40];
	sprintf(filename, "/dev/i2c-1");

	if ( (file = open(filename, O_RDWR)) < 0)
	{
		printf("Failed to open the bus.");
		return -1;
	}
	if (ioctl(file, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return -1;
	}

	return file;
}

int i2cMem8Read(int dev, int add, uint8_t* buff, int size)
{
	uint8_t intBuff[I2C_SMBUS_BLOCK_MAX];

	if (NULL == buff)
	{
		return -1;
	}

	if (size > I2C_SMBUS_BLOCK_MAX)
	{
		return -1;
	}

	intBuff[0] = 0xff & add;

	if (write(dev, intBuff, 1) != 1)
	{
		//printf("Fail to select mem add!\n");
		return -1;
	}
	if (read(dev, buff, size) != size)
	{
		//printf("Fail to read memory!\n");
		return -1;
	}
	return 0; //OK
}

int i2cMem8Write(int dev, int add, uint8_t* buff, int size)
{
	uint8_t intBuff[I2C_SMBUS_BLOCK_MAX];

	if (NULL == buff)
	{
		return -1;
	}

	if (size > I2C_SMBUS_BLOCK_MAX - 1)
	{
		return -1;
	}

	intBuff[0] = 0xff & add;
	memcpy(&intBuff[1], buff, size);

	if (write(dev, intBuff, size + 1) != size + 1)
	{
		//printf("Fail to write memory!\n");
		return -1;
	}
	return 0;
}

modbus_t *modbusSetup(int boardAddress)
{
	modbus_t *ctx = NULL;
	int dev = 0;
	ModbusSetingsType settings;
	u8 buff[5];
	int byteSize = 8;
	char parity;
    uint32_t sec_to = 1;
    uint32_t usec_to = 0;
	int i = 0, i2c_stack = 0;

	for (i = 0; i < 8; i++)
	{
		if (boardCheck(RELAY3_HW_I2C_BASE_ADD + i) == OK)
		{
			i2c_stack = i;
		}
	}

#ifdef DEBUG_MODBUS
	printf("stack level i2c: %d\r\n", i2c_stack);
#endif

	i2c_stack = i2c_stack + RELAY3_HW_I2C_BASE_ADD;
	dev = i2cSetup(i2c_stack);

	if (dev == -1)
	{
		return NULL;
	}

	if (OK != i2cMem8Read(dev, I2C_MODBUS_SETINGS_ADD, buff, 5))
	{
		printf("Fail to read RS485 settings!\n");
		return NULL;
	}

	memcpy(&settings, buff, sizeof(ModbusSetingsType));

	switch ((int)settings.mbParity)
	{
		case 0:
			parity = 'N';
			break;
		case 1:
			parity = 'E';
			break;
		case 2:
			parity = 'O';
			break;
		default:
			parity = 'N';
			break;
	}

#ifdef DEBUG_MODBUS
	printf("parity = %c\r\n", parity);
#endif

	ctx = modbus_new_rtu(UART_PORT, (int)settings.mbBaud, parity, byteSize, (int)settings.mbStopB);

    if (ctx == NULL)
    {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return NULL;
    }


#ifdef DEBUG_MODBUS
	if (modbus_set_debug(ctx, TRUE) == ERROR)
	{
		return NULL;
	}
#endif

	if (modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL) == ERROR ||
		modbus_set_slave(ctx, boardAddress + 1) == ERROR ||
		modbus_get_response_timeout(ctx, &sec_to, &usec_to) == ERROR)
	{
		printf("Fail to set slave ID!\n");
		return NULL;
	}

	modbus_enable_rpi(ctx, TRUE);

	if (modbus_connect(ctx) == ERROR)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return ERROR;
    }

	return ctx;
}

int modbusRead(modbus_t *ctx, int relayRegisterAddress, uint8_t *relayReadState)
{
	int numberOfBits = 1;

	if (ctx == NULL)
	{
		printf("Modbus context NULL!\r\n");
		return ERROR;
	}

	if (modbus_read_bits(ctx, relayRegisterAddress, numberOfBits, relayReadState) == ERROR)
	{
		return ERROR;
	}

	return OK;
}

int modbusWrite(modbus_t *ctx, int relayRegisterAddress, uint8_t relayWriteState)
{
	if (ctx == NULL)
	{
		printf("Modbus context NULL!\r\n");
		return ERROR;
	}

	if (modbus_write_bit(ctx, relayRegisterAddress, relayWriteState) == ERROR)
	{
		return ERROR;
	}

	return OK;
}
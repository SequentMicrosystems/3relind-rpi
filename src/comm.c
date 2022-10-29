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
#include <stdlib.h>
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

rs485_settings_t modbusGetSettings(char modbusSettingsStr[], int index)
{
	rs485_settings_t modbusRs485Settings;
	int j = 0, k = 0;
	char bufStr[255];

	modbusRs485Settings.index = index;
	memset(modbusRs485Settings.bufStr, 0, 255*sizeof(char));
	memset(bufStr, 0, 255*sizeof(char));

	while(modbusSettingsStr[modbusRs485Settings.index] != ',')
	{
		if (modbusSettingsStr[modbusRs485Settings.index] == '.')
			break;
		else
			modbusRs485Settings.index++;
	}

	j = modbusRs485Settings.index;
	while(modbusSettingsStr[j] != ' ')
		j--;

	j++;
	while(modbusSettingsStr[j] != ',')
	{
		if (modbusSettingsStr[j] == '.')
			break;
		else
		{
			bufStr[k] = modbusSettingsStr[j];
			j++;
			k++;
		}
	}

	memcpy(modbusRs485Settings.bufStr, bufStr, strlen(bufStr));
#ifdef DEBUG_MODBUS
	printf("strlen(bufStr) = %d\r\n", strlen(bufStr));
	printf("bufStr = %s\r\n", bufStr);
	printf("modbusRs485Settings.bufStr = %s\r\n", modbusRs485Settings.bufStr);
#endif

	return modbusRs485Settings;
}

modbus_t *modbusSetup(int boardAddress)
{
	modbus_t *ctx = NULL;
	rs485_settings_t modbusRs485Settings;
	char modbusSettingsStr[255];
	int index = 0;
	int modbusBaud = 0;
	char modbusParity = '0';
	int modbusByteSize = MODBUS_DEFAULT_BYTESIZE;
	int modbusStopBits = 0;
	uint32_t sec_to = 1;
	uint32_t usec_to = 0;
	FILE *fp = NULL;

	modbusRs485Settings.index = 0;
	memset(modbusRs485Settings.bufStr, 0, 255*sizeof(char));
	memset(modbusSettingsStr, 0, 255*sizeof(char));

	/* try to open modbus settings file */
	fp = fopen("/usr/local/etc/modbus_rs485_settings.txt", "r");
	if (fp == NULL)
	{
		/* use default settings */
#ifdef DEBUG_MODBUS
		printf("'modbus_rs485_settings.txt' does not exist. Using default settings...\r\n");
#endif
		modbusBaud = MODBUS_DEFAULT_BAUD;
		modbusParity = MODBUS_DEFAULT_PARITY;
		modbusStopBits = MODBUS_DEFAULT_STOPBITS;

#ifdef DEBUG_MODBUS
		printf("modbusBaud = %d\r\n", modbusBaud);
		printf("modbusParity = ");
		putchar(modbusParity);
		printf("\r\n");
		printf("modbusStopBits = %d\r\n", modbusStopBits);
#endif

		/* create file and write settings with defaults */
		fp = fopen("/usr/local/etc/modbus_rs485_settings.txt", "w");
		if (fp == NULL)
		{
			printf("Error while creating modbus settings file. Using default settings...\r\n");
			modbusBaud = MODBUS_DEFAULT_BAUD;
			modbusParity = MODBUS_DEFAULT_PARITY;
			modbusStopBits = MODBUS_DEFAULT_STOPBITS;
		}
		else
		{
			/* write default settings to the new file */
			snprintf(modbusSettingsStr, sizeof(modbusSettingsStr), "[MODBUS Settings] Baud_rate: %d, Parity: %c, Stop_bits: %d.", modbusBaud, modbusParity, modbusStopBits);
			fprintf(fp, "%s", modbusSettingsStr);
			fclose(fp);
		}
	}
	else
	{
		/* read settings from file */
		fgets(modbusSettingsStr, sizeof(modbusSettingsStr), fp);
		if (modbusSettingsStr == NULL || ferror(fp))
		{
			/* use default settings */
			printf("Error while reading 'modbus_rs485_settings.txt'. Using default settings...\r\n");
			modbusBaud = MODBUS_DEFAULT_BAUD;
			modbusParity = MODBUS_DEFAULT_PARITY;
			modbusStopBits = MODBUS_DEFAULT_STOPBITS;
			fclose(fp);
		}
		else
		{
			/* read settings from file and use them */
#ifdef DEBUG_MODBUS
			printf("Reading modbus settings from file...\r\n");
			printf("%s\r\n", modbusSettingsStr);
#endif
			// find first setting (modbusBaud)
			modbusRs485Settings = modbusGetSettings(modbusSettingsStr, index);
			modbusBaud = atoi(modbusRs485Settings.bufStr);

			// find 2nd setting (modbusParity)
			modbusRs485Settings = modbusGetSettings(modbusSettingsStr, modbusRs485Settings.index + 1);
			modbusParity = modbusRs485Settings.bufStr[0];

			// find 3rd setting (modbusStopBits)
			modbusRs485Settings = modbusGetSettings(modbusSettingsStr, modbusRs485Settings.index + 1);
			modbusStopBits = atoi(modbusRs485Settings.bufStr);

			fclose(fp);

#ifdef DEBUG_MODBUS
			printf("modbusBaud = %d\r\n", modbusBaud);
			printf("modbusParity = ");
			putchar(modbusParity);
			printf("\r\n");
			printf("modbusStopBits = %d\r\n", modbusStopBits);
#endif
		}
	}

	ctx = modbus_new_rtu(MODBUS_UART_PORT, modbusBaud, modbusParity, modbusByteSize, modbusStopBits);

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
		modbus_free(ctx);
		return NULL;
	}

	modbus_enable_rpi(ctx, TRUE);

	if (modbus_connect(ctx) == ERROR)
	{
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return NULL;
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
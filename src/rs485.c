#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "relay.h"
#include "comm.h"

int rs485Set(int dev, u8 mode, u32 baud, u8 stopB, u8 parity, u8 add)
{
	ModbusSetingsType settings;
	u8 buff[5];

	if (baud > 920600 || baud < 1200)
	{
   if(mode == 0)
   {
     baud = 9600;
   }
   else
   {
		printf("Invalid RS485 Baudrate [1200, 920600]!\n");
		return ERROR;
   }
	}
	if (mode > 1)
	{
		printf("Invalid RS485 mode : 0 = disable, 1= Modbus RTU (Slave)!\n");
		return ERROR;
	}
	if (stopB < 1 || stopB > 2)
	{
   if(mode == 0 )
   {
    stopB = 1;
   }
   else
   {
		printf("Invalid RS485 stop bits [1, 2]!\n");
		return ERROR;
   }
	}
	if (parity > 2)
	{
		printf("Invalid RS485 parity 0 = none; 1 = even; 2 = odd! Set to none.\n");
		parity = 0;
	}
	if (add < 1)
	{
		  printf("Invalid MODBUS device address: [1, 255]! Set to 1.\n");
     add = 1;
	}
	settings.mbBaud = baud;
	settings.mbType = mode;
	settings.mbParity = parity;
	settings.mbStopB = stopB;
	settings.add = add;

	memcpy(buff, &settings, sizeof(ModbusSetingsType));
	if (OK != i2cMem8Write(dev, I2C_MODBUS_SETINGS_ADD, buff, 5))
	{
		printf("Fail to write RS485 settings!\n");
		return ERROR;
	}
	return OK;
}

int rs485Get(int dev)
{
	ModbusSetingsType settings;
	u8 buff[5];

	if (OK != i2cMem8Read(dev, I2C_MODBUS_SETINGS_ADD, buff, 5))
	{
		printf("Fail to read RS485 settings!\n");
		return ERROR;
	}
	memcpy(&settings, buff, sizeof(ModbusSetingsType));
	printf("<mode> <baudrate> <stopbits> <parity> <add> %d %d %d %d %d\n",
		(int)settings.mbType, (int)settings.mbBaud, (int)settings.mbStopB,
		(int)settings.mbParity, (int)settings.add);
	return OK;
}

int doRs485Read(int argc, char *argv[]);
const CliCmdType CMD_RS485_READ =
{
	"cfg485rd",
	2,
	&doRs485Read,
	"\tcfg485rd:    Read the RS485 communication settings\n",
	"\tUsage:       3relind <id> cfg485rd\n",
	"",
	"\tExample:     3relind 0 cfg485rd; Read the RS485 settings on Board #0\n"};

int doRs485Read(int argc, char *argv[])
{
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 3)
	{
		if (OK != rs485Get(dev))
		{
			return ERROR;
		}
	}
	else
	{
		printf("Usage: 3relind <id> cfg485rd\n");
		return ARG_CNT_ERR;
	}
	return OK;
}

int doRs485Write(int argc, char *argv[]);
const CliCmdType CMD_RS485_WRITE =
{
	"cfg485wr",
	2,
	&doRs485Write,
	"\tcfg485wr:    Write the RS485 communication settings\n",
	"\tUsage:       3relind <id> cfg485wr <mode> <baudrate> <stopBits> <parity> <slaveAddr>\n",
	"",
	"\tExample:     3relind 0 cfg485wr 1 9600 1 0 1; Write the RS485 settings on Board #0\n\t\t     (mode = Modbus RTU; baudrate = 9600 bps; stop bits one; parity none; modbus slave address = 1)\n"};

int doRs485Write(int argc, char *argv[])
{
	int dev = 0;
	u8 mode = 0;
	u32 baud = 1200;
	u8 stopB = 1;
	u8 parity = 0;
	u8 add = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 8)
	{
		mode = 0xff & atoi(argv[3]);
		baud = atoi(argv[4]);
		stopB = 0xff & atoi(argv[5]);
		parity = 0xff & atoi(argv[6]);
		add = 0xff & atoi(argv[7]);
		if (OK != rs485Set(dev, mode, baud, stopB, parity, add))
		{
			return ERROR;
		}
		printf("done\n");
	}
	else
	{
		printf("Usage: 3relind <id> cfg485wr <mode> <baudrate> <stopBits> <parity> <slaveAddr>\n");
		return ARG_CNT_ERR;
	}
	return OK;
}

int rs485ModbusSet(int modbusBaud, int parity, int modbusStopBits)
{
	FILE *fp = NULL;
	char modbusSettingsStr[255];
	char modbusParity = '0';

	memset(modbusSettingsStr, 0, 255*sizeof(char));

	if (modbusBaud > 920600 || modbusBaud < 1200)
	{
		printf("Invalid RS485 Modbus Baudrate [1200, 920600]!\n");
		return ERROR;
	}

	if (parity > 2 || parity < 0)
	{
		printf("Invalid RS485 Modbus parity 0 = none; 1 = even; 2 = odd!\n");
		return ERROR;
	}

	if (modbusStopBits > 2 || modbusStopBits < 1)
	{
		printf("Invalid RS485 Modbus stop bits [1, 2]!\n");
		return ERROR;
	}

	/* write settings to file */
	fp = fopen("/usr/local/etc/modbus_rs485_settings.txt", "w");
	if (fp == NULL)
	{
		printf("Error while writing to 'modbus_rs485_settings.txt'!\r\n");
		return ERROR;
	}
	else
	{
		switch (parity)
		{
			case 0:
				modbusParity = 'N';
				break;
			case 1:
				modbusParity = 'E';
				break;
			case 2:
				modbusParity = 'O';
				break;
			default:
				modbusParity = 'N';
				break;
		}
		snprintf(modbusSettingsStr, sizeof(modbusSettingsStr), "[MODBUS Settings] Baud_rate: %d, Parity: %c, Stop_bits: %d.", modbusBaud, modbusParity, modbusStopBits);
		fprintf(fp, "%s", modbusSettingsStr);
		fclose(fp);
	}
	return OK;
}

int rs485ModbusGet()
{
	FILE *fp = NULL;
	char modbusSettingsStr[255];
	rs485_settings_t modbusRs485Settings;
	int index = 0;
	int modbusBaud = 0;
	char modbusParity = '0';
	int modbusParityInt = 0;
	int modbusStopBits = 0;

	modbusRs485Settings.index = 0;
	memset(modbusRs485Settings.bufStr, 0, 255*sizeof(char));
	memset(modbusSettingsStr, 0, 255*sizeof(char));

	/* try to open modbus settings file */
	fp = fopen("/usr/local/etc/modbus_rs485_settings.txt", "r");
	if (fp == NULL)
	{
		printf("'modbus_rs485_settings.txt' does not exist!\r\n");
		return ERROR;
	}
	else
	{
		/* read settings from file and check */
		fgets(modbusSettingsStr, sizeof(modbusSettingsStr), fp);
		if (modbusSettingsStr == NULL || ferror(fp))
		{
			printf("Error while reading 'modbus_rs485_settings.txt'!\r\n");
			fclose(fp);
			return ERROR;
		}
		else
		{
			/* print settings */
#ifdef DEBUG_MODBUS
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

			switch (modbusParity)
			{
				case 'N':
					modbusParityInt = 0;
					break;
				case 'E':
					modbusParityInt = 1;
					break;
				case 'O':
					modbusParityInt = 2;
					break;
				default:
					modbusParityInt = 0;
					break;
			}

			printf("<baudrate> <parity> <stopbits> %d %d %d\n", modbusBaud, modbusParityInt, modbusStopBits);
			fclose(fp);
		}
	}
	return OK;
}

int doRs485ModbusRead(int argc, char *argv[]);
const CliCmdType CMD_RS485_MODBUS_READ =
{
	"cfgmbrd",
	1,
	&doRs485ModbusRead,
	"\tcfgmbrd:     Read the Modbus RS485 communication settings\n",
	"\tUsage:       3relind cfgmbrd\n",
	"",
	"\tExample:     3relind cfgmbrd; Read the Modbus RS485 settings\n"};

int doRs485ModbusRead(int argc, char *argv[])
{
	if (argc == 2)
	{
		if (OK != rs485ModbusGet())
		{
			return ERROR;
		}
	}
	else
	{
		printf("Usage: 3relind cfgmbrd\n");
		return ARG_CNT_ERR;
	}
	return OK;
}

int doRs485ModbusWrite(int argc, char *argv[]);
const CliCmdType CMD_RS485_MODBUS_WRITE =
{
	"cfgmbwr",
	1,
	&doRs485ModbusWrite,
	"\tcfgmbwr:     Write the Modbus RS485 communication settings\n",
	"\tUsage:       3relind cfgmbwr <baudrate> <parity> <stopBits>\n",
	"",
	"\tExample:     3relind cfgmbwr 9600 0 1; Write the Modbus RS485 settings\n\t\t     (baudrate = 9600 bps; parity none; stop bits one)\n\t\t     Default settings: baudrate = 38400 bps; parity none; stop bits one\n"};

int doRs485ModbusWrite(int argc, char *argv[])
{
	int baud = 0;
	int stopB = 0;
	int parity = 0;

	if (argc == 5)
	{
		baud = atoi(argv[2]);
		parity = atoi(argv[3]);
		stopB = atoi(argv[4]);

		if (OK != rs485ModbusSet(baud, parity, stopB))
		{
			return ERROR;
		}

		printf("done\n");
	}
	else
	{
		printf("Usage: 3relind cfgmbwr <baudrate> <parity> <stopBits>\n");
		return ARG_CNT_ERR;
	}
	return OK;
}
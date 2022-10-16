/*
 * relay.c:
 *	Command-line interface to the Raspberry
 *	Pi's 8-Relay Industrial board.
 *	Copyright (c) 2016-2021 Sequent Microsystem
 *	<http://www.sequentmicrosystem.com>
 ***********************************************************************
 *	Author: Alexandru Burcea
 ***********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "relay.h"
#include "comm.h"
#include "thread.h"

// testing modbus after first build
#include "modbus.h"
#include <errno.h>
#include <unistd.h>

#define VERSION_BASE	(int)1
#define VERSION_MAJOR	(int)1
#define VERSION_MINOR	(int)1

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#define CMD_ARRAY_SIZE	11

int relayChSet(int dev, u8 channel, OutStateEnumType state);
int relayChGet(int dev, u8 channel, OutStateEnumType* state);

static int doHelp(int argc, char *argv[]);
const CliCmdType CMD_HELP =
	{
		"-h",
		1,
		&doHelp,
		"\t-h          Display the list of command options or one command option details\n",
		"\tUsage:      3relind -h    Display command options list\n",
		"\tUsage:      3relind -h <param>   Display help for <param> command option\n",
		"\tExample:    3relind -h write    Display help for \"write\" command option\n"};

static int doVersion(int argc, char *argv[]);
const CliCmdType CMD_VERSION =
{
	"-v",
	1,
	&doVersion,
	"\t-v              Display the version number\n",
	"\tUsage:          3relind -v\n",
	"",
	"\tExample:        3relind -v  Display the version number\n"};

static int doWarranty(int argc, char* argv[]);
const CliCmdType CMD_WAR =
{
	"-warranty",
	1,
	&doWarranty,
	"\t-warranty       Display the warranty\n",
	"\tUsage:          3relind -warranty\n",
	"",
	"\tExample:        3relind -warranty  Display the warranty text\n"};

static int doList(int argc, char *argv[]);
const CliCmdType CMD_LIST =
	{
		"-list",
		1,
		&doList,
		"\t-list:       List all 3relind boards connected,\n\treturn       nr of boards and stack level for every board\n",
		"\tUsage:       3relind -list\n",
		"",
		"\tExample:     3relind -list display: 1,0 \n"};

static int doRelayWrite(int argc, char *argv[]);
const CliCmdType CMD_WRITE =
{
	"write",
	2,
	&doRelayWrite,
	"\twrite:       Set relays On/Off\n",
	"\tUsage:       3relind <id> write <channel> <on/off>\n",
	"\tUsage:       3relind <id> write <value>\n",
	"\tExample:     3relind 0 write 2 On; Set Relay #2 on Board #0 On\n"};

static int doRelayRead(int argc, char *argv[]);
const CliCmdType CMD_READ =
{
	"read",
	2,
	&doRelayRead,
	"\tread:        Read relays status\n",
	"\tUsage:       3relind <id> read <channel>\n",
	"\tUsage:       3relind <id> read\n",
	"\tExample:     3relind 0 read 2; Read Status of Relay #2 on Board #0\n"};

static int doTest(int argc, char* argv[]);
const CliCmdType CMD_TEST =
{
	"test",
	2,
	&doTest,
	"\ttest:        Turn ON and OFF the relays until press a key\n",
	"",
	"\tUsage:       3relind <id> test\n",
	"\tExample:     3relind 0 test\n"};

CliCmdType gCmdArray[CMD_ARRAY_SIZE];

char *usage = "Usage:	 3relind -h <command>\n"
	"         3relind -v\n"
	"         3relind -warranty\n"
	"         3relind -list\n"
	"         3relind <id> write <channel> <on/off>\n"
	"         3relind <id> write <value>\n"
	"         3relind <id> read <channel>\n"
	"         3relind <id> read\n"
	"         3relind <id> test\n"
	"         3relind <id> rs485rd\n"
	"         3relind <id> rs485wr\n"
	"Where: <id> = Board level id = 0..7\n"
	"Type 3relind -h <command> for more help"; // No trailing newline needed here.

char *warranty =
	"	       Copyright (c) 2016-2020 Sequent Microsystems\n"
		"                                                             \n"
		"		This program is free software; you can redistribute it and/or modify\n"
		"		it under the terms of the GNU Leser General Public License as published\n"
		"		by the Free Software Foundation, either version 3 of the License, or\n"
		"		(at your option) any later version.\n"
		"                                    \n"
		"		This program is distributed in the hope that it will be useful,\n"
		"		but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"		GNU Lesser General Public License for more details.\n"
		"			\n"
		"		You should have received a copy of the GNU Lesser General Public License\n"
		"		along with this program. If not, see <http://www.gnu.org/licenses/>.";

int relayChSet(int dev, u8 channel, OutStateEnumType state)
{
	int resp;
	u8 buff[2];

	if ( (channel < CHANNEL_NR_MIN) || (channel > RELAY_CH_NR_MAX))
	{
		printf("Invalid relay nr!\n");
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL, buff, 1))
	{
		return FAIL;
	}

	switch (state)
	{
	case OFF:
		buff[0] &= ~ (1 << (channel - 1));
		resp = i2cMem8Write(dev, I2C_MEM_RELAY_VAL, buff, 1);
		break;
	case ON:
		buff[0] |= 1 << (channel - 1);
		resp = i2cMem8Write(dev, I2C_MEM_RELAY_VAL, buff, 1);
		break;
	default:
		printf("Invalid relay state!\n");
		return ERROR;
		break;
	}
	return resp;
}

int relayChGet(int dev, u8 channel, OutStateEnumType* state)
{
	u8 buff[2];

	if (NULL == state)
	{
		return ERROR;
	}

	if ( (channel < CHANNEL_NR_MIN) || (channel > RELAY_CH_NR_MAX))
	{
		printf("Invalid relay nr!\n");
		return ERROR;
	}

	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL, buff, 1))
	{
		return ERROR;
	}

	if (buff[0] & (1 << (channel - 1)))
	{
		*state = ON;
	}
	else
	{
		*state = OFF;
	}
	return OK;
}

int relaySet(int dev, int val)
{
	u8 buff[2];

	buff[0] = 0xff & val;

	return i2cMem8Write(dev, I2C_MEM_RELAY_VAL, buff, 1);
}

int relayGet(int dev, int* val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL, buff, 1))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

int doBoardInit(int stack)
{
	int dev = 0;
	int add = 0;
	uint8_t buff[8];

	if ( (stack < 0) || (stack > 7))
	{
		printf("Invalid stack level [0..7]!");
		return ERROR;
	}
	add = stack + RELAY3_HW_I2C_BASE_ADD;
	dev = i2cSetup(add);
	if (dev == -1)
	{
		return ERROR;

	}
	if (ERROR == i2cMem8Read(dev, I2C_MEM_RELAY_VAL, buff, 1))
	{
		printf("3relind board id %d not detected\n", stack);
		return ERROR;
	}
	return dev;
}

int doBoardModbusInit(int stack)
{
	return OK;
}

int boardCheck(int hwAdd)
{
	int dev = 0;
	uint8_t buff[8];

	dev = i2cSetup(hwAdd);
	if (dev == -1)
	{
		return FAIL;
	}
	if (ERROR == i2cMem8Read(dev, I2C_MEM_RELAY_VAL, buff, 1))
	{
		return ERROR;
	}
	return OK;
}

int boardModbusCheck(int hwAdd)
{
	return OK;
}

/*
 * doRelayWrite:
 *	Write coresponding relay channel
 **************************************************************************************
 */
static int doRelayWrite(int argc, char *argv[])
{
	int pin = 0;
	OutStateEnumType state = STATE_COUNT;
	int val = 0;
	int dev = 0;
	OutStateEnumType stateR = STATE_COUNT;
	int valR = 0;
	int retry = 0;

	if ( (argc != 5) && (argc != 4))
	{
		printf("Usage: 3relind <id> write <relay number> <on/off> \n");
		printf("Usage: 3relind <id> write <relay reg value> \n");
		return ARG_CNT_ERR;
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return COMM_ERR;
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > RELAY_CH_NR_MAX))
		{
			printf("Relay number value out of range\n");
			return ARG_CNT_ERR;
		}

		/**/if ( (strcasecmp(argv[4], "up") == 0)
			|| (strcasecmp(argv[4], "on") == 0))
			state = ON;
		else if ( (strcasecmp(argv[4], "down") == 0)
			|| (strcasecmp(argv[4], "off") == 0))
			state = OFF;
		else
		{
			if ( (atoi(argv[4]) >= STATE_COUNT) || (atoi(argv[4]) < 0))
			{
				printf("Invalid relay state!\n");
				return ARG_CNT_ERR;
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		retry = RETRY_TIMES;

		while ( (retry > 0) && (stateR != state))
		{
			if (OK != relayChSet(dev, pin, state))
			{
				printf("Fail to write relay\n");
				return COMM_ERR;
			}
			if (OK != relayChGet(dev, pin, &stateR))
			{
				printf("Fail to read relay\n");
				return COMM_ERR;
			}
			retry--;
		}
#ifdef DEBUG_I
		if(retry < RETRY_TIMES)
		{
			printf("retry %d times\n", 3-retry);
		}
#endif
		if (retry == 0)
		{
			printf("Fail to write relay\n");
			return COMM_ERR;
		}
	}
	else
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 255)
		{
			printf("Invalid relay value\n");
			return ARG_CNT_ERR;
		}

		retry = RETRY_TIMES;
		valR = -1;
		while ( (retry > 0) && (valR != val))
		{

			if (OK != relaySet(dev, val))
			{
				printf("Fail to write relay!\n");
				return COMM_ERR;
			}
			if (OK != relayGet(dev, &valR))
			{
				printf("Fail to read relay!\n");
				return COMM_ERR;
			}
		}
		if (retry == 0)
		{
			printf("Fail to write relay!\n");
			return COMM_ERR;
		}
	}
	return OK;
}

#if 0

/*
 * doRelayMWrite:
 *	Write coresponding modbus rtu relay channel
 **************************************************************************************
 */
static int doRelayMWrite(int argc, char *argv[])
{
	printf("test mwrite command!\r\n");
    
	modbus_t *ctx = NULL;
    uint32_t sec_to = 1;
    uint32_t usec_to = 0;
	uint8_t dest = 0;

    int i;
    int rc;
    int nb_points = 1;

	// new modbus
    ctx = modbus_new_rtu(UART_PORT, BAUD_RATE, PARITY, BYTESIZE, STOPBITS);
    if (ctx == NULL)
    {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }


	// modbus init
    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);
    modbus_set_slave(ctx, SERVER_ID);
    modbus_get_response_timeout(ctx, &sec_to, &usec_to);
    modbus_enable_rpi(ctx,TRUE);

	// modbus connect
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }


	// start the first test

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS, 1);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 1, 1);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 1, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 2, 1);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 2, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(3);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS, 0);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 1, 0);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 1, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 2, 0);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 2, 1, &dest);
    printf("read bits: %d\n", dest);


    sleep(1);



    modbus_set_slave(ctx, SERVER_ID + 4);


    sleep(1);



    rc = modbus_write_bit(ctx, REGISTER_ADDRESS, 1);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 1, 1);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 1, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 2, 1);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 2, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(3);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS, 0);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 1, 0);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 1, 1, &dest);
    printf("read bits: %d\n", dest);

    sleep(2);

    rc = modbus_write_bit(ctx, REGISTER_ADDRESS + 2, 0);
    printf("rc : %d\n", rc);

    rc = modbus_read_bits(ctx, REGISTER_ADDRESS + 2, 1, &dest);
    printf("read bits: %d\n", dest);

	// close connection
    modbus_close(ctx);
    modbus_free(ctx);


	return OK;
}

#endif


/*
 * doRelayRead:
 *	Read relay state
 ******************************************************************************************
 */
static int doRelayRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;
	OutStateEnumType state = STATE_COUNT;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return COMM_ERR;
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > RELAY_CH_NR_MAX))
		{
			printf("Relay number value out of range!\n");
			return ARG_CNT_ERR;
		}

		if (OK != relayChGet(dev, pin, &state))
		{
			printf("Fail to read!\n");
			return COMM_ERR;
		}
		if (state != 0)
		{
			printf("1\n");
		}
		else
		{
			printf("0\n");
		}
	}
	else if (argc == 3)
	{
		if (OK != relayGet(dev, &val))
		{
			printf("Fail to read!\n");
			return COMM_ERR;
		}
		printf("%d\n", val);
	}
	else
	{
		printf("Usage: %s read relay value\n", argv[0]);
		return ARG_CNT_ERR;
	}
	return OK;
}

static int doHelp(int argc, char *argv[])
{
	int i = 0;
	if (argc == 3)
	{
		for (i = 0; i < CMD_ARRAY_SIZE; i++)
		{
			if ( (gCmdArray[i].name != NULL))
			{
				if (strcasecmp(argv[2], gCmdArray[i].name) == 0)
				{
					printf("%s%s%s%s", gCmdArray[i].help, gCmdArray[i].usage1,
						gCmdArray[i].usage2, gCmdArray[i].example);
					break;
				}
			}
		}
		if (CMD_ARRAY_SIZE == i)
		{
			printf("Option \"%s\" not found\n", argv[2]);
			printf("%s: %s\n", argv[0], usage);
		}
	}
	else
	{
		printf("%s: %s\n", argv[0], usage);
	}
	return OK;
}

static int doVersion(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	printf("3relind v%d.%d.%d Copyright (c) 2016 - 2022 Sequent Microsystems\n",
	VERSION_BASE, VERSION_MAJOR, VERSION_MINOR);
	printf("\nThis is free software with ABSOLUTELY NO WARRANTY.\n");
	printf("For details type: 3relind -warranty\n");
	return OK;
}

static int doList(int argc, char *argv[])
{
	int ids[8];
	int i;
	int cnt = 0;

	UNUSED(argc);
	UNUSED(argv);

	for (i = 0; i < 8; i++)
	{
		if (boardCheck(RELAY3_HW_I2C_BASE_ADD + i) == OK)
		{
			ids[cnt] = i;
			cnt++;
		}

	}
	printf("%d board(s) detected\n", cnt);
	if (cnt > 0)
	{
		printf("Id:");
	}
	while (cnt > 0)
	{
		cnt--;
		printf(" %d", ids[cnt]);
	}
	printf("\n");
	return OK;
}

/* 
 * Self test for production
 */
static int doTest(int argc, char* argv[])
{
	int dev = 0;
	int i = 0;
	int retry = 0;
	int relVal;
	int valR;
	int relayResult = 0;
	FILE* file = NULL;
	const u8 relayOrder[8] =
	{
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	if (argc == 4)
	{
		file = fopen(argv[3], "w");
		if (!file)
		{
			printf("Fail to open result file\n");
			//return -1;
		}
	}
//relay test****************************
	if (strcasecmp(argv[2], "test") == 0)
	{
		relVal = 0;
		printf(
			"Are all relays and LEDs turning on and off in sequence?\nPress y for Yes or any key for No....");
		startThread();
		while (relayResult == 0)
		{
			for (i = 0; i < 3; i++)
			{
				relayResult = checkThreadResult();
				if (relayResult != 0)
				{
					break;
				}
				valR = 0;
				relVal = (u8)1 << (relayOrder[i] - 1);

				retry = RETRY_TIMES;
				while ( (retry > 0) && ( (valR & relVal) == 0))
				{
					if (OK != relayChSet(dev, relayOrder[i], ON))
					{
						retry = 0;
						break;
					}

					if (OK != relayGet(dev, &valR))
					{
						retry = 0;
					}
				}
				if (retry == 0)
				{
					printf("Fail to write relay\n");
					if (file)
						fclose(file);
					return COMM_ERR;
				}
				busyWait(150);
			}

			for (i = 0; i < 3; i++)
			{
				relayResult = checkThreadResult();
				if (relayResult != 0)
				{
					break;
				}
				valR = 0xff;
				relVal = (u8)1 << (relayOrder[i] - 1);
				retry = RETRY_TIMES;
				while ( (retry > 0) && ( (valR & relVal) != 0))
				{
					if (OK != relayChSet(dev, relayOrder[i], OFF))
					{
						retry = 0;
					}
					if (OK != relayGet(dev, &valR))
					{
						retry = 0;
					}
				}
				if (retry == 0)
				{
					printf("Fail to write relay!\n");
					if (file)
						fclose(file);
					return COMM_ERR;
				}
				busyWait(150);
			}
		}
	}
	if (relayResult == YES)
	{
		if (file)
		{
			fprintf(file, "Relay Test ............................ PASS\n");
		}
		else
		{
			printf("Relay Test ............................ PASS\n");
		}
	}
	else
	{
		if (file)
		{
			fprintf(file, "Relay Test ............................ FAIL!\n");
		}
		else
		{
			printf("Relay Test ............................ FAIL!\n");
		}
	}
	if (file)
	{
		fclose(file);
	}
	relaySet(dev, 0);
	return OK;
}

static int doWarranty(int argc UNU, char* argv[] UNU)
{
	printf("%s\n", warranty);
	return OK;
}

static void cliInit(void)
{
	int i = 0;

	memset(gCmdArray, 0, sizeof(CliCmdType) * CMD_ARRAY_SIZE);

	memcpy(&gCmdArray[i], &CMD_HELP, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_WAR, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_LIST, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_WRITE, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_TEST, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_VERSION, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_RS485_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_RS485_WRITE, sizeof(CliCmdType));
	i++;

}

int main(int argc, char *argv[])
{
	int i = 0;

	cliInit();

	if (argc == 1)
	{
		printf("%s\n", usage);
		return 1;
	}
	for (i = 0; i < CMD_ARRAY_SIZE; i++)
	{
		if ( (gCmdArray[i].name != NULL) && (gCmdArray[i].namePos < argc))
		{
			if (strcasecmp(argv[gCmdArray[i].namePos], gCmdArray[i].name) == 0)
			{
				gCmdArray[i].pFunc(argc, argv);
				return 0;
			}
		}
	}
	printf("Invalid command option\n");
	printf("%s\n", usage);

	return 0;
}

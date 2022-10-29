#ifndef RELAY3_H_
#define RELAY3_H_

#include <stdint.h>
#include "modbus.h"

#define RETRY_TIMES	10
enum
{
	I2C_MEM_RELAY_VAL = 0,//reserved 4 bits for open-drain and 4 bits for leds
	I2C_MEM_RELAY_SET,
	I2C_MEM_RELAY_CLR,

	I2C_MEM_DIAG_3V,
	I2C_MEM_DIAG_3V_1,
	I2C_MEM_TEMP,
	I2C_MODBUS_SETINGS_ADD,
	I2C_NBS1,
	I2C_MBS2,
	I2C_MBS3,
	I2C_MODBUS_ID_OFFSET_ADD,
	I2C_MEM_REVISION_MAJOR = 0x78,
	I2C_MEM_REVISION_MINOR,


	I2C_MEM_CPU_RESET = 0xaa,

	SLAVE_BUFF_SIZE = 255
};

#define CHANNEL_NR_MIN		1
#define RELAY_CH_NR_MAX		3
#define MODBUS_RELAY_CH_NR_MIN		0
#define MODBUS_RELAY_CH_NR_MAX		2

#define ERROR	-1
#define OK		0
#define FAIL	-1
#define ARG_CNT_ERR -3
#define COMM_ERR -4

#define RELAY3_HW_I2C_BASE_ADD	0x60

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef enum
{
	OFF = 0,
	ON,
	STATE_COUNT
} OutStateEnumType;

typedef struct
{
 const char* name;
 const int namePos;
 int(*pFunc)(int, char**);
 const char* help;
 const char* usage1;
 const char* usage2;
 const char* example;
}CliCmdType;


typedef struct
	__attribute__((packed))
	{
		unsigned int mbBaud :24;
		unsigned int mbType :4;
		unsigned int mbParity :2;
		unsigned int mbStopB :2;
		unsigned int add:8;
	} ModbusSetingsType;
	
// RS-485 CLI structures
extern const CliCmdType CMD_RS485_READ;
extern const CliCmdType CMD_RS485_WRITE;
extern const CliCmdType CMD_RS485_MODBUS_READ;
extern const CliCmdType CMD_RS485_MODBUS_WRITE;

int doBoardInit(int stack);
int boardCheck(int hwAdd);
modbus_t *doBoardModbusInit(int stack);
#endif //RELAY3_H_

#ifndef RELAY3_H_
#define RELAY3_H_

#include <stdint.h>

#define RETRY_TIMES	10
enum
{
	I2C_MEM_RELAY_VAL = 0,//reserved 4 bits for open-drain and 4 bits for leds
	I2C_MEM_RELAY_SET,
	I2C_MEM_RELAY_CLR,

	I2C_MEM_REVISION_MAJOR = 0x78,
	I2C_MEM_REVISION_MINOR,

	I2C_MEM_DIAG_3V,
	I2C_MEM_DIAG_3V_1,

	I2C_MEM_CPU_RESET = 0xaa,

	SLAVE_BUFF_SIZE = 255
};

#define CHANNEL_NR_MIN		1
#define RELAY_CH_NR_MAX		3

#define ERROR	-1
#define OK		0
#define FAIL	-1

#define RELAY3_HW_I2C_BASE_ADD	0x60
typedef uint8_t u8;
typedef uint16_t u16;

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
 void(*pFunc)(int, char**);
 const char* help;
 const char* usage1;
 const char* usage2;
 const char* example;
}CliCmdType;

#endif //RELAY3_H_

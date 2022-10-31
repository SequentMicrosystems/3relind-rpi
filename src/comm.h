#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <errno.h>
#include "modbus.h"
#include "relay.h"

#define MODBUS_UART_PORT            "/dev/ttyS0"
#define MODBUS_DEFAULT_BAUD         38400
#define MODBUS_DEFAULT_PARITY       'N'
#define MODBUS_DEFAULT_STOPBITS     1
#define MODBUS_DEFAULT_BYTESIZE     8
#undef DEBUG_MODBUS

typedef struct rs485_settings
{
	char bufStr[255];
	int  index;
} rs485_settings_t;

int i2cSetup(int addr);
int i2cMem8Read(int dev, int add, uint8_t* buff, int size);
int i2cMem8Write(int dev, int add, uint8_t* buff, int size);

modbus_t *modbusSetup(int boardAddress);
int modbusRead(modbus_t *ctx, int relayRegisterAddress, uint8_t *relayReadState);
int modbusWrite(modbus_t *ctx, int relayRegisterAddress, uint8_t relayWriteState);
rs485_settings_t modbusGetSettings(char modbusSettingsStr[], int index);

#endif //COMM_H_
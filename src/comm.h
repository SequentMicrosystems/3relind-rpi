#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <errno.h>
#include "modbus.h"
#include "relay.h"

#define UART_PORT "/dev/ttyS0"
#undef DEBUG_MODBUS

int i2cSetup(int addr);
int i2cMem8Read(int dev, int add, uint8_t* buff, int size);
int i2cMem8Write(int dev, int add, uint8_t* buff, int size);

modbus_t *modbusSetup(int boardAddress);
int modbusRead(modbus_t *ctx, int relayRegisterAddress, uint8_t *relayReadState);
int modbusWrite(modbus_t *ctx, int relayRegisterAddress, uint8_t relayWriteState);

#endif //COMM_H_
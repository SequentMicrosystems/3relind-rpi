#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <errno.h>
#include "modbus.h"
#include "relay.h"

#define UART_PORT "/dev/ttyS0"
#define DEBUG_MODBUS 1

int i2cSetup(int addr);
int i2cMem8Read(int dev, int add, uint8_t* buff, int size);
int i2cMem8Write(int dev, int add, uint8_t* buff, int size);

int modbusSetup(int boardAddress, modbus_t *ctx);
int modbusRead(int relayRegisterAddress, modbus_t *ctx, uint8_t *relayReadState);
int modbusWrite(int relayRegisterAddress, modbus_t *ctx, uint8_t relayWriteState);

#endif //COMM_H_
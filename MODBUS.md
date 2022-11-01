[![3relind-rpi](res/sequent.jpg)](https://sequentmicrosystems.com)

# Modbus

The [Three 40A/240V Relays RS485 Daisy-chainable HAT for Raspberry Pi]((https://sequentmicrosystems.com/collections/all-io-cards/products/three-40a-240v-relays-rs485-for-raspberry-pi)) can be accessed thru Modbus RTU protocol over RS-485 port.
You can set-up the RS-485 port with **3relind** command.

Example:
```bash
~$ 3relind 0 cfg485wr 1 9600 1 0 1
```
Set Modbus RTU , Baudrate: 9600bps, 1 Stop Bit,  parity: None, slave address base: 1
```bash
~$ 3relind -h rs485wr
```
display the full set of options

## Slave Address
The five address switches have a dual purpose. 
When the card is accessed from Raspberry Pi, the switches define the offset of the I2C address, permitting changing the adress to avoid adress conflict on the bus.
When the card is accessed through MODBUS protocol over the RS485 port, the switches define the offset of the RS485 base address set by the software.

## Modbus object types
All modbus RTU object type with standard addresses are implemented : Coils, Discrete Inputs, Input registers, Holding registers.

### Coils

Acces level Read/Write, Size 1 bit

| Device function | Register Address | Modbus Address |
| --- | --- | --- |
| COIL_RELAY1 | 0001 | 0x00 |
|	COIL_RELAY2 | 0002 | 0x01 |
|	COIL_RELAY3| 0003 | 0x02 |

### Discrete Inputs

NONE

### Input registers

Access level Read Only, Size 16 bits

| Device function | Register Address | Modbus Address | Measurement Unit |
| --- | --- | --- | --- |
| IR_TEMP | 30001 | 0x00 | C |



### Holding registers

Access level Read/Write, Size 16 bits

NONE

## Function codes implemented

* Read Coils (0x01)
* Read Discrete Inputs (0x02)
* Read Holding Registers (0x03)
* Read Input Registers (0x04)
* Write Single Coil (0x05)
* Write Single Register (0x06)
* Write Multiple Coils (0x0f)
* Write Multiple registers (0x10)

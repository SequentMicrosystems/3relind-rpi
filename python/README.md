
[![3relind-rpi](res/sequent.jpg)](https://sequentmicrosystems.com)

# lib3relind

This is the python library to control the [Three 40A/240V Relays RS485 Daisy-chainable HAT for Raspberry Pi](https://sequentmicrosystems.com/collections/all-io-cards/products/three-40a-240v-relays-rs485-for-raspberry-pi).

## Install

```bash
~$ sudo apt-get update
~$ sudo apt-get install build-essential python-pip python-dev python-setuptools git
~$ sudo pip install -U minimalmodbus
~$ sudo pip install -U smbus
~$ git clone https://github.com/SequentMicrosystems/3relind-rpi.git
~$ cd 3relind-rpi/python/3relind/
~/3relind-rpi/python/3relind$ sudo python setup.py install
```
For python3.x:
```bash
~$ sudo apt-get update
~$ sudo apt-get install build-essential python3-pip python3-dev python3-smbus git
~$ sudo pip3 install -U minimalmodbus
~$ git clone https://github.com/SequentMicrosystems/3relind-rpi.git
~$ cd 3relind-rpi/python/3relind/
~/3relind-rpi/python/3relind$ sudo python3 setup.py install
```
## Update

```bash
~$ cd 3relind-rpi/
~/3relind-rpi$ git pull
~$ cd 3relind-rpi/python/3relind/
~/3relind-rpi/python/3relind$ sudo python setup.py install
```
For python3.x the last line must be replaced with:
```bash
~/3relind-rpi/python/3relind$ sudo python3 setup.py install
```
## Usage 

Now you can import the lib3relind library and use its functions. To test, read relays status from the board with address 0:

```bash
~$ python
Python 2.7.9 (default, Sep 17 2016, 20:26:04)
[GCC 4.9.2] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> import lib3relind
>>> lib3relind.get_all(0)
0
>>>
```

## Functions

### set(address, relay, value)
Set one relay state.

address - address of the 3-Relay card (selectable from address microswitches [0..7])

relay - relay number (id) [1..3]

value - relay state 1: turn ON, 0: turn OFF[0..1]


### set_all(address, value)
Set all relays state.

address - address of the 3-Relay card (selectable from address microswitches [0..7])

value - 3 bit value of all relays (ex: 7: turn on all relays, 0: turn off all relays, 1:turn on relay #1 and off the rest)

### get(address, relay)
Get one relay state.

address - address of the 3-Relay card (selectable from address microswitches [0..7])

relay - relay number (id) [1..3]

return 0 == relay off; 1 - relay on

### get_all(address)
Return the state of all relays.

address - address of the 3-Relay card (selectable from address microswitches [0..7])

return - [0..7]

### mset(boardAddress, relay, value, baudrate = 38400, parity = 0, stopbits = 1)
Set one modbus relay state.

boardAddress - address of the 3-Relay card (selectable from address microswitches [1..254])

relay - relay number (id) [1..3]

value - relay state 1: turn ON, 0: turn OFF[0..1]

baudrate - baud rate parameter for RS485 interface (default: 38400)

parity - parity parameter for RS485 interface (default: 0 - None)

stopbits - stop bits parameter for RS485 interface (default: 1)


### mset_all(boardAddress, value, baudrate = 38400, parity = 0, stopbits = 1)
Set all modbus relays state.

boardAddress - address of the 3-Relay card (selectable from address microswitches [1..254])

value - 3 bit value of all relays (ex: 255: turn on all relays, 0: turn off all relays, 1:turn on relay #1 and off the rest)

baudrate - baud rate parameter for RS485 interface (default: 38400)

parity - parity parameter for RS485 interface (default: 0 - None)

stopbits - stop bits parameter for RS485 interface (default: 1)

### mget(boardAddress, relay, baudrate = 38400, parity = 0, stopbits = 1)
Get one modbus relay state.

boardAddress - address of the 3-Relay card (selectable from address microswitches [1..254])

relay - relay number (id) [1..3]

baudrate - baud rate parameter for RS485 interface (default: 38400)

parity - parity parameter for RS485 interface (default: 0 - None)

stopbits - stop bits parameter for RS485 interface (default: 1)

return 0 == relay off; 1 - relay on

### mget_all(boardAddress, baudrate = 38400, parity = 0, stopbits = 1)
Return the state of all modbus relays.

boardAddress - address of the 3-Relay card (selectable from address microswitches [1..254])

baudrate - baud rate parameter for RS485 interface (default: 38400)

parity - parity parameter for RS485 interface (default: 0 - None)

stopbits - stop bits parameter for RS485 interface (default: 1)

return - [0..7]

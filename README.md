[![3relind-rpi](res/sequent.jpg)](https://sequentmicrosystems.com)

[![3relind-rpi](res/3-rel1.jpg)](https://sequentmicrosystems.com/collections/all-io-cards/products/three-40a-240v-relays-rs485-for-raspberry-pi)

# 3relind-rpi
Drivers for Sequent Microsystems [Three 40A/240V Relays RS485 Daisy-chainable HAT for Raspberry Pi](https://sequentmicrosystems.com/collections/all-io-cards/products/three-40a-240v-relays-rs485-for-raspberry-pi)

## Setup (first usage of the card)
 - Enable the I2C communication:
```bash
~$ sudo raspi-config
```
Go to the "Interface Options">>"I2C" menu and select "Yes"

 - Enable UART communication:
```bash
~$ sudo raspi-config
```
Go to the "Interface Options">>"Serial Port".

Select "No" for login sell acces over serial.

Select "Yes" for enable serial port hardware.

 - Update your Raspberry PI and install the tools you need:
 ```bash
~$ sudo apt-get udate
~$ sudo apt-get upgrade
~$ sudo apt-get install git
```

## Install
```bash
~$ git clone https://github.com/SequentMicrosystems/3relind-rpi.git
~$ cd 3relind-rpi/
~/3relind-rpi$ sudo make install
```

Now you can access all the functions of the relays board through the command "3relind". Use -h option for help:
```bash
~$ 3relind -h
```
## Update
If you clone the repository, any update can be made with the following commands:

```bash
~$ cd 3relind-rpi/  
~/3relind-rpi$ git pull
~/3relind-rpi$ sudo make install
```  
## Usage
The ```3relind``` command has 2 sets of options, one for accessing the cards connected directly to Raspberry through I2C and one for accessing the cards connected through RS485, using Modbus RTU protocol.
For accessing the cards over RS485 you need to use one of the connected card's RS485 transceivers and for that, you must do a few setup steps:
* Disable the Modbus interface for the card connected directly to the Raspberry PI, example for a card with stack level 0: ```3relind 0 cfg485wr 0 38400 1 0 1```
* Turn on the microswitches marked "Tx" and "Rx" on the card connected to the Raspberry PI
* (Optional) Set the Raspberry UART port parameters by issuing the command ```3relind cfgmbwr <baudrate> <parity> <stopBits>```. The step is optional because the default parameters are loaded (38400 N 1).
* (Optional) Set the communications parameters for the remote card. Connect the card directly to the Raspberry and run the command ```3relind 0 cfg485wr 1 38400 1 0 1``` then disconnect the card. This step is optional because by default the settings are: Communication protocol: Modbus RTU; Baudrate: 38400; Stop bits: 1; Parity: None; Modbus address base 1.


Many thanks to [stephane](https://github.com/stephane/libmodbus) and [dhruvvyas90](https://github.com/dhruvvyas90/libmodbus) for their great work on libmodbus software library and for letting us use it in our project.

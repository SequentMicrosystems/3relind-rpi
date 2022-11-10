# node-red-contrib-sm-8relind

This is the node-red node to control Sequent Microsystems [Three 40A/240V Relays RS485 Daisy-chainable HAT for Raspberry Pi](https://sequentmicrosystems.com/collections/all-io-cards/products/three-40a-240v-relays-rs485-for-raspberry-pi).

## Manual Install

Clone or update the repository, follow the instructions from the [first page.](https://github.com/SequentMicrosystems/3relind-rpi)

In your node-red user directory, tipicaly ~/.node-red,

```bash
~$ cd ~/.node-red
```

Run the following command:

```bash
~/.node-red$ npm install ~/3relind-rpi/node-red-contrib-sm-3relind
```

In order to see the node in the palette and use it you need to restart node-red. If you run node-red as a service:
 ```bash
 ~$ node-red-stop
 ~& node-red-start
 ```

## Usage

After install and restart the node-red you will see on the node palete, under Sequent Microsystems category the "3relind" and "3relindrd" nodes.

### 3relind node
This node will turn on or off a relay or all relays as a 3 bits bitmap. The card address and relay number can be set in the dialog screen or dinamicaly thru ``` msg.address``` and ``` msg.relay ```. The output of one relay or all 3 relays if you set the relay number to 0, can be set dynamically using  ``` msg.payload ```.

### 3relindrd node
This node will read one relay state or all relays states as a 3 bits bitmap. The card address and relay number can be set in the dialog screen or dinamicaly thru ``` msg.address``` and ``` msg.relay ``` and the state is output as  ``` msg.payload ``` .If you set the relay number to 0 the node will output the state of all relays.

## Important note

This node is using the I2C-bus package from @fivdi, you can visit his work on github [here](https://github.com/fivdi/i2c-bus).
The inspiration for this node came from @nielsnl68 work with [node-red-contrib-i2c](https://github.com/nielsnl68/node-red-contrib-i2c). We thank them for the great job.

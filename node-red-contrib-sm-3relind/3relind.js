module.exports = function(RED) {
    "use strict";
    var I2C = require("i2c-bus");
    const RELAY3_HW_I2C_BASE_ADD = 0x60;
    const I2C_MEM_RELAY_VAL = 0x00;

    // The relay Node
    function RelayNode(n) {
        RED.nodes.createNode(this, n);
        this.address = parseInt(n.address);
        this.relay = parseInt(n.relay);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;

        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var address = node.address;
            var hwAdd = RELAY3_HW_I2C_BASE_ADD;
            var relay = node.relay;
            var relayVal = 0;

            if (isNaN(address)) address = msg.address;
            address = parseInt(address);

            if (isNaN(relay)) relay = msg.relay;
            relay = parseInt(relay);

            if (isNaN(address + 1)) {
                this.status({fill:"red",shape:"ring",text:"Address ("+address+") value is missing or incorrect"});
                return;
            } else if (isNaN(relay) ) {
                this.status({fill:"red",shape:"ring",text:"Relay number ("+relay+") value is missing or incorrect"});
                return;
            } else {
                this.status({});
            }

            if(address < 0){
              address = 0;
            }
            if(address > 7){
              address = 7;
            }

            hwAdd += address;

            try {
                if (this.payloadType == null) {
                    myPayload = this.payload;
                } else if (this.payloadType == 'none') {
                    myPayload = null;
                } else {
                    myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
                }

                if(relay < 0){
                  relay = 0;
                }
                if(relay > 3){
                  relay = 3;
                }

                if(relay > 0)
                {
                  relayVal = node.port.readByteSync(hwAdd, I2C_MEM_RELAY_VAL);
                  relay-= 1;
                  if (myPayload == null || myPayload == false || myPayload == 0 || myPayload == 'off') {
                    /* set relay off */
                    relayVal &= ~ (1 << relay);
                  } else {
                    /* set relay on */
                    relayVal |= 1 << relay;
                  }
                }
                else if(myPayload >= 0 && myPayload < 256)
                {
                  /* set value for all relays */
                  relayVal = 0xFF & myPayload;
                }
                node.port.writeByte(hwAdd, I2C_MEM_RELAY_VAL, relayVal, function(err) {
                    if (err) {
                      node.error(err, msg);
                    } else {
                      node.send(msg);
                    }
                });
            } catch(err) {
                this.error(err,msg);
            }
        });
        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("3relind", RelayNode);

    // The relay read Node
    function RelayReadNode(n) {
        RED.nodes.createNode(this, n);
        this.address = parseInt(n.address);
        this.relay = parseInt(n.relay);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;

        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var address = node.address;
            var relay = node.relay;
            var hwAdd = RELAY3_HW_I2C_BASE_ADD;
            var relayVal = 0;

            if (isNaN(address)) address = msg.address;
            address = parseInt(address);

            if (isNaN(relay)) relay = msg.relay;
            relay = parseInt(relay);

            if (isNaN(address + 1)) {
                this.status({fill:"red",shape:"ring",text:"Address ("+address+") value is missing or incorrect"});
                return;
            } else if (isNaN(relay) ) {
                this.status({fill:"red",shape:"ring",text:"Relay number ("+relay+") value is missing or incorrect"});
                return;
            } else {
                this.status({});
            }

            if(address < 0){
                address = 0;
            }
            if(address > 7){
              address = 7;
            }

            hwAdd += address;

            try {
                if (this.payloadType == null) {
                    myPayload = this.payload;
                } else if (this.payloadType == 'none') {
                    myPayload = null;
                } else {
                    myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
                }

                relayVal = node.port.readByteSync(hwAdd, I2C_MEM_RELAY_VAL);

                if(relay < 0){
                  relay = 0;
                }
                if(relay > 3){
                  relay = 3;
                }

                if(relay > 0){
                  /* return the state of a single relay */
                  relay-= 1;
                  if (relayVal & (1 << relay))
                  {
                    msg.payload = 1;
                  }
                  else
                  {
                    msg.payload = 0;
                  }
                }
                else {
                  /* return the value of all relays */
                  msg.payload = relayVal;
                }
                node.send(msg);
            } catch(err) {
                this.error(err,msg);
            }
        });
        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("3relindrd", RelayReadNode);
}

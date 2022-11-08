import smbus
import serial
import minimalmodbus

# bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)

DEVICE_ADDRESS = 0x60  # 7 bit address (will be left shifted to add the read write bit)

RELAY3_INPORT_REG_ADD = 0x00


def __check(bus, add):
    return bus.read_byte_data(add, RELAY3_INPORT_REG_ADD)


def DecToBin(value):
    binary = [1 if digit=='1' else 0 for digit in bin(value)[2:]]
    binary.reverse()
    zero = [0, 0, 0]
    if len(binary) < 3:
        zero[:len(binary)] = binary
        binary = zero
    elif len(binary) > 3:
        binary = binary[0:3]
    return binary


def BinToDec(binary):
    return sum(val*(2**idx) for idx, val in enumerate(binary))


def __setup_modbus(boardAddress, baudrate, parity, stopbits):
    if boardAddress < 1 or boardAddress > 254:
        raise ValueError('Invalid modbus board address!')
    if baudrate < 1200 or baudrate > 920600:
        raise ValueError('Invalid modbus baudrate!')
    if parity < 0 or parity > 2:
        raise ValueError('Invalid modbus parity!')
    if stopbits < 1 or stopbits > 2:
        raise ValueError('Invalid modbus stop bits!')
    board = minimalmodbus.Instrument('/dev/serial0', boardAddress, debug = False)
    board.serial.baudrate = baudrate
    board.serial.bytesize = 8
    board.serial.stopbits = stopbits
    if parity == 0:
        board.serial.parity = serial.PARITY_NONE
    if parity == 1:
        board.serial.parity = serial.PARITY_EVEN
    if parity == 2:
        board.serial.parity = serial.PARITY_ODD
    board.mode = minimalmodbus.MODE_RTU
    board.clear_buffers_before_each_transaction = True
    return board


def __check_modbus_board(board, relay = 0):
    try:
        value = board.read_bit(relay, functioncode = 1)
    except Exception as e:
        board.close_port_after_each_call = True
        raise ValueError('3-relay card not detected!')
    return value


def set(address, relay, value):
    if address < 0 or address > 7:
        raise ValueError('Invalid board address!')
    if relay < 1:
        raise ValueError('Invalid relay number!')
    if relay > 3:
        raise ValueError('Invalid relay number!')
    if value < 0 or value > 1:
        raise ValueError('Invalid relay value!')
    bus = smbus.SMBus(1)
    hwAdd = DEVICE_ADDRESS + address
    try:
        oldVal = __check(bus, hwAdd)
    except Exception as e:
        bus.close()
        raise ValueError('3-relay card not detected!')
    try:
        if value == 0:
            oldVal = oldVal & (~(1 << (relay - 1)))
            bus.write_byte_data(hwAdd, RELAY3_INPORT_REG_ADD, oldVal)
        else:
            oldVal = oldVal | (1 << (relay - 1))
            bus.write_byte_data(hwAdd, RELAY3_INPORT_REG_ADD, oldVal)
    except Exception as e:
        bus.close()
        raise ValueError('Fail to write relay state value!')
    bus.close()


def set_all(address, value):
    if address < 0 or address > 7:
        raise ValueError('Invalid board address!')
    if value > 255:
        raise ValueError('Invalid relay value!')
    if value < 0:
        raise ValueError('Invalid relay value!')
    bus = smbus.SMBus(1)
    hwAdd = DEVICE_ADDRESS + address
    try:
        oldVal = __check(bus, hwAdd)
    except Exception as e:
        bus.close()
        raise ValueError('3-relay card not detected!')
    try:
        bus.write_byte_data(hwAdd, RELAY3_INPORT_REG_ADD, value)
    except Exception as e:
        bus.close()
        raise ValueError('Fail to write relay state value!')
    bus.close()


def get(address, relay):
    if address < 0 or address > 7:
        raise ValueError('Invalid board address!')
    if relay < 1:
        raise ValueError('Invalid relay number!')
    if relay > 3:
        raise ValueError('Invalid relay number!')
    bus = smbus.SMBus(1)
    hwAdd = DEVICE_ADDRESS + address
    try:
        val = __check(bus, hwAdd)
    except Exception as e:
        bus.close()
        raise ValueError('3-relay card not detected!')
    val = val & (1 << (relay - 1))
    bus.close()
    if val == 0:
        return 0
    else:
        return 1


def get_all(address):
    if address < 0 or address > 7:
        raise ValueError('Invalid board address!')
    bus = smbus.SMBus(1)
    hwAdd = DEVICE_ADDRESS + address
    try:
        val = __check(bus, hwAdd)
    except Exception as e:
        bus.close()
        raise ValueError('3-relay card not detected!')
    bus.close()
    return val


def mset(boardAddress, relay, value, baudrate = 38400, parity = 0, stopbits = 1):
    if relay < 1 or relay > 3:
        raise ValueError('Invalid relay number!')
    if value < 0 or value > 1:
        raise ValueError('Invalid relay value!')
    relay = relay - 1
    board = __setup_modbus(boardAddress, baudrate, parity, stopbits)
    __check_modbus_board(board)
    try:
        board.write_bit(relay, value, functioncode = 5)
    except Exception as e:
        board.close_port_after_each_call = True
        raise ValueError('Fail to write relay state value!')
    board.close_port_after_each_call = True


def mset_all(boardAddress, value, baudrate = 38400, parity = 0, stopbits = 1):
    if value < 0 or value > 255:
        raise ValueError('Invalid relay value!')
    board = __setup_modbus(boardAddress, baudrate, parity, stopbits)
    __check_modbus_board(board)
    valueBin = DecToBin(value)
    try:
        board.write_bits(0, valueBin)
    except Exception as e:
        board.close_port_after_each_call = True
        raise ValueError('Fail to write relay state value!')
    board.close_port_after_each_call = True


def mget(boardAddress, relay, baudrate = 38400, parity = 0, stopbits = 1):
    if relay < 1 or relay > 3:
        raise ValueError('Invalid relay number!')
    relay = relay - 1
    board = __setup_modbus(boardAddress, baudrate, parity, stopbits)
    value = __check_modbus_board(board, relay)
    board.close_port_after_each_call = True
    if value == 0:
        return 0
    else:
        return 1


def mget_all(boardAddress, baudrate = 38400, parity = 0, stopbits = 1):
    board = __setup_modbus(boardAddress, baudrate, parity, stopbits)
    __check_modbus_board(board)
    try:
        valueBin = board.read_bits(0, 3, functioncode = 1)
    except Exception as e:
        board.close_port_after_each_call = True
        raise ValueError('Fail to read relay state values!')
    board.close_port_after_each_call = True
    value = BinToDec(valueBin)
    return value
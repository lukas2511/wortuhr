import usb.core
import sys
import time

def uint8_to_uint32(buf):
    return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

def uint32_to_uint8(buf):
    return [(buf & 0x000000FF), (buf & 0x0000FF00) >> 8, (buf & 0x00FF0000) >> 16, (buf & 0xFF000000) >> 24]

if __name__ == "__main__":
    dev = usb.core.find(idVendor=0x2511, idProduct=0x1337)
    if dev is None:
        raise ValueError('Device not found')

    dev.set_configuration()

    if len(sys.argv) > 1 and sys.argv[1] == '-s':
        if not dev.ctrl_transfer(0x40, 1, 42   , 0, uint32_to_uint8(int(time.time()))) == 4:
            print("uuuh wtf?!")
            sys.exit(1)

    buf = dev.ctrl_transfer(0xC0, 1, 42   , 0, 4)
    unixtime = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
    print(unixtime)

#!/usr/bin/env python2

"""
Script sends 'send status' command to the USB bridge.
"""

import sys
from time import sleep
import usb.core
import usb.util

# Bulk Vendor HID device VID and PID
device_vid = 0x03EB
device_pid = 0x206C
device_in_ep = 2
device_out_ep = 1

def get_vendor_device_handle():
	dev_handle = usb.core.find(idVendor = device_vid, idProduct = device_pid)
	return dev_handle

def write(device, packet):
    device.write(usb.util.ENDPOINT_OUT | device_out_ep, packet, 0, 1000)
    # print("Sent Packet: {0}".format(packet))


def read(device):
    packet = device.read(usb.util.ENDPOINT_IN | device_in_ep, 64, 0, 1000)
    print("Received Packet: {0}".format(packet))

    return packet


def main():
    vendor_device = get_vendor_device_handle()

    if vendor_device is None:
        print("No valid Vendor device found.")
        sys.exit(1)

    vendor_device.set_configuration()

    print("Connected to device 0x%04X/0x%04X - %s [%s]" %
          (vendor_device.idVendor, vendor_device.idProduct,
           usb.util.get_string(vendor_device, 255, vendor_device.iProduct),
           usb.util.get_string(vendor_device, 255, vendor_device.iManufacturer)))

    while 1:
        l = range(128)
        l[0] = 201
        l[1] = 222
        write(vendor_device, l)
        # write(vendor_device, [201, 222]);
        read(vendor_device)
        # sleep(1)

if __name__ == '__main__':
    main()

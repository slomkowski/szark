#!/usr/bin/env python2

"""
Receives camera image.
"""

import json
import socket
import sys
import time

import cv2
import numpy


def print_usage():
    print("Usage:\n"
          "%s {host} {port} [input identifiers]\n\n"
          "press 's' to save image, 'q' to exit, space switches between inputs" % __file__)


if len(sys.argv) < 3:
    print_usage()
    sys.exit(1)

udpAddress = sys.argv[1]
udpPort = int(sys.argv[2])

input_identifiers = sys.argv[3:] if len(sys.argv) > 3 else ['default']
current_input_no = 0
current_serial = 1

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.7)

print_usage()

while True:
    req_header = {
        "serial": current_serial,
        "compress": False,
        "drawHud": True,
        "input": input_identifiers[current_input_no]
    }
    current_serial += 1

    try:
        sock.sendto(json.dumps(req_header), (udpAddress, udpPort))
        (data, addr) = sock.recvfrom(60000)
    except socket.timeout:
        continue

    for _headerEnd in range(max(len(data), 300)):
        if ord(data[_headerEnd]) == 0:
            headerEnd = _headerEnd
            break

    header = json.loads(data[:headerEnd])

    img = cv2.imdecode(numpy.fromstring(data[headerEnd + 1:], dtype='uint8'), 1)
    cv2.imshow('Camera view', img)

    k = cv2.waitKey(10)
    if k == ord('s'):
        with open('dump-%d.jpg' % time.time(), 'wb') as f:
            f.write(data[headerEnd + 1:])
    elif k == ord(' '):
        current_input_no = (current_input_no + 1) % len(input_identifiers)
        print("Switched input to %s." % input_identifiers[current_input_no])
    elif k == 27 or k == ord('q'):
        cv2.destroyAllWindows()
        break

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

udpAddress = sys.argv[1]
udpPort = int(sys.argv[2])

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

req_header = {
    "serial": 5,
    "compress": False,
    "drawHud": True
}

while True:
    sock.sendto(json.dumps(req_header), (udpAddress, udpPort))
    (data, addr) = sock.recvfrom(60000)

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
    elif k == 27 or k == ord('q'):
        cv2.destroyAllWindows()
        break

#!/usr/bin/env python2

"""
Receives camera image.
"""

import sys
import socket
import cv2
import numpy

udpAddress = sys.argv[1]
udpPort = int(sys.argv[2])

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    sock.sendto('RAW'.encode('utf-8'), (udpAddress, udpPort))
    (data, addr) = sock.recvfrom(50000)

    img = cv2.imdecode(numpy.fromstring(data, dtype='uint8'), 1)
    cv2.imshow('Camera view', img)

    k = cv2.waitKey(10)
    if k == 27:
        cv2.destroyAllWindows()
        break

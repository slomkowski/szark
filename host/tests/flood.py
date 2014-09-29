#!/usr/bin/env python

"""
Script sends the JSON command to the server indefinetely.
"""

import sys
import json
import socket

request = { "killswitch": False, "serial" : 0, "lcd" : "hello world xfdsfs" }


udpAddress = '127.0.0.1'
udpPort = 10191

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

jsonText = json.dumps(request)
print("Flooding " + str(sys.argv[1]) + " with requests...")

i = 0
while True:
    jsonText = json.dumps(request)
    sock.sendto(jsonText.encode("UTF-8"), (sys.argv[1], udpPort))
    request['serial'] = i
    i = i + 1



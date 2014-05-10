#!/usr/bin/env python

"""
Script sends the JSON command to the server many times.
"""

import json
import socket

request = { "killswitch": False, "serial" : 0, "motor" : { "left" : { "dir" : "forward", "speed":4}}}

udpAddress = '127.0.0.1'
udpPort = 10191

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

jsonText = json.dumps(request)
print("JSON: " + jsonText)

for i in range(5000):
    jsonText = json.dumps(request)
    sock.sendto(jsonText.encode("UTF-8"), (udpAddress, udpPort))
    request['serial'] = i


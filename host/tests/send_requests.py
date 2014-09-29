#!/usr/bin/env python

"""
Script sends the JSON command to the server many times.
"""

import sys
import json
import socket

if sys.argv[2] == "false":
    val = False
else:
    val = True

#request = { "killswitch": False, "serial" : 0, "motor" : { "left" : { "dir" : sys.argv[2], "speed":int(sys.argv[3])}}}
#request = { "killswitch": False, "serial" : 0, "arm" : { "elbow" : { "dir" : sys.argv[2], "speed":int(sys.argv[3])}}}
request = { "killswitch": False, "serial" : 0, "light" : { "left" : val}}
#request = { "killswitch": False, "serial" : 0, "lcd" : "hello world xfdsfs" }


udpAddress = '127.0.0.1'
udpPort = 10191

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

jsonText = json.dumps(request)
print("JSON: " + jsonText)

for i in range(int(sys.argv[1])):
    jsonText = json.dumps(request)
    sock.sendto(jsonText.encode("UTF-8"), (udpAddress, udpPort))
    request['serial'] = i
    #request['lcd'] =request['lcd'] + str(i)



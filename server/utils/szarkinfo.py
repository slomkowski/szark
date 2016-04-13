#!/usr/bin/env python3

"""
Queries running SZARK servers.
"""

import json
import socket
import sys

CONTROL_SERVER_PORT = 10191
CAMERA_SERVER_PORT = 10192


def print_usage():
    print("Usage:\n" + "%s [host]\n")


def display_control_info(resp):
    return """
* Emergency stop status: %s
* Battery: %.1f V, current: %.1f A""" % (resp['ks_stat'], resp['b']['u'], resp['b']['i'])


def display_camera_info(header, image):
    return "* Returned image: %.2f kB" % (len(image) / 1000.0)


def query_control_server(address):
    req_header = {
        "serial": 0,
    }

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(0.7)
        sock.sendto(json.dumps(req_header).encode('utf-8'), (address, CONTROL_SERVER_PORT))
        (data, addr) = sock.recvfrom(1000)
        response = json.loads(data.decode('utf-8'))
        return response
    except socket.timeout:
        return None


def query_camera_server(address):
    req_header = {
        "serial": 0
    }

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(0.7)
        sock.sendto(json.dumps(req_header).encode('utf-8'), (address, CAMERA_SERVER_PORT))
        (data, addr) = sock.recvfrom(0xffff)

        for _headerEnd in range(max(len(data), 300)):
            if data[_headerEnd] == 0:
                headerEnd = _headerEnd
                break

        header = json.loads(data[:headerEnd].decode('utf-8'))
        image = data[headerEnd:]

        return header, image
    except socket.timeout:
        return None


if __name__ == "__main__":
    udpAddress = sys.argv[1] if len(sys.argv) > 1 else "localhost"

    print("Querying control server at", udpAddress, "on port", CONTROL_SERVER_PORT, "...")

    control_serv_resp = query_control_server(udpAddress)

    print("Querying camera server at", udpAddress, "on port", CAMERA_SERVER_PORT, "...")

    cam_serv_resp = query_camera_server(udpAddress)

    if control_serv_resp is not None:
        print(display_control_info(control_serv_resp))
    else:
        print("!! Control server not available!")

    if cam_serv_resp is not None:
        print(display_camera_info(*cam_serv_resp))
    else:
        print("!! Camera server not available!")

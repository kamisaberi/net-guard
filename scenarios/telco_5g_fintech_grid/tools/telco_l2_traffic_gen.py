#!/usr/bin/env python3
# File: /home/kami/net-guard/scenarios/telco_5g_fintech_grid/tools/telco_l2_traffic_gen.py

import socket
import struct
import time


def main():
    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    raw_sock = None
    try:
        raw_sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x88B8))
        raw_sock.bind(("eth0", 0))
        print("[+] Telco/L2 Wire Generator: AF_PACKET Non-IP L2 (0x88B8 GOOSE) bound to eth0", flush=True)
    except Exception as exc:
        print(f"[-] AF_PACKET raw L2 bind deferred: {exc}", flush=True)

    sq = 1
    while True:
        # 1. 5G URLLC GTP-U (:2152) -> Edge-5G-UPF-01
        gtpu = struct.pack(">BBHI", 0x30, 0xFF, 24, 0x00000100) + b"\x45\x00\x00\x18" + b"\x00" * 5 + b"\x06" + b"\x00" * 10 + struct.pack(">HH", 12345, 502)
        try:
            udp_sock.sendto(gtpu, ("10.240.60.101", 2152))
        except Exception:
            pass

        # 2. Non-IP Layer-2 IEC 61850 GOOSE (0x88B8) -> multicast 01:0C:CD:01:00:01
        if raw_sock is not None:
            l2_frame = b"\x01\x0c\xcd\x01\x00\x01\x02\x42\x0a\xf0\x3c\x32\x88\xb8" + struct.pack(">HHHHII", 0x0001, 16, 0, 0, 10, sq)
            try:
                raw_sock.send(l2_frame)
            except Exception:
                pass

        # 3. FinTech ITCH (:9001) -> Edge-FinTech-ITCH-03
        itch = b"A" + (b"\x00" * 19) + struct.pack(">I", 100) + (b"\x00" * 8) + struct.pack(">I", 1425000)
        try:
            udp_sock.sendto(itch, ("10.240.60.103", 9001))
        except Exception:
            pass

        sq = (sq % 65000) + 1
        time.sleep(0.5)


if __name__ == "__main__":
    main()
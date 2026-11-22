#!/usr/bin/env python3
# File: /home/kami/net-guard/scenarios/telco_5g_fintech_grid/tools/generate_telco_pcaps.py

import struct
import time
from pathlib import Path

PCAP_DIR = Path(__file__).resolve().parent.parent / "pcaps"
PCAP_DIR.mkdir(parents=True, exist_ok=True)


def write_pcap(path: Path, frames: list[bytes], linktype: int = 1):
    with open(path, "wb") as f:
        f.write(struct.pack("<IHHIIII", 0xA1B2C3D4, 2, 4, 0, 0, 65535, linktype))
        t0 = int(time.time())
        for idx, frame in enumerate(frames):
            f.write(struct.pack("<IIII", t0 + idx // 10, (idx % 10) * 100000, len(frame), len(frame)))
            f.write(frame)


def wrap_eth_ipv4_udp(src_ip: str, dst_ip: str, sport: int, dport: int, payload: bytes) -> bytes:
    eth = b"\x02\x42\x0a\xf0\x3c\x65\x02\x42\x0a\xf0\x3c\x32\x08\x00"
    udp_len = 8 + len(payload)
    ip_len = 20 + udp_len
    s_b = bytes(map(int, src_ip.split(".")))
    d_b = bytes(map(int, dst_ip.split(".")))
    ip = struct.pack(">BBHHHBBH4s4s", 0x45, 0, ip_len, 1, 0, 64, 17, 0, s_b, d_b)
    udp = struct.pack(">HHHH", sport, dport, udp_len, 0)
    return eth + ip + udp + payload


def main():
    gtpu_nom = struct.pack(">BBHI", 0x30, 0xFF, 24, 0x00000100) + b"\x45\x00\x00\x18" + b"\x00" * 5 + b"\x06" + b"\x00" * 10 + struct.pack(">HH", 12345, 502)
    gtpu_hop = struct.pack(">BBHI", 0x30, 0xFF, 24, 0x00000200) + b"\x45\x00\x00\x18" + b"\x00" * 5 + b"\x06" + b"\x00" * 10 + struct.pack(">HH", 12345, 502)
    write_pcap(PCAP_DIR / "gtpu_nominal.pcap", [wrap_eth_ipv4_udp("10.240.60.50", "10.240.60.101", 2152, 2152, gtpu_nom) for _ in range(10)])
    write_pcap(PCAP_DIR / "gtpu_slice_hop_attack.pcap", [wrap_eth_ipv4_udp("10.240.60.99", "10.240.60.101", 2152, 2152, gtpu_hop) for _ in range(10)])

    goose_eth = b"\x01\x0c\xcd\x01\x00\x01\x02\x42\x0a\xf0\x3c\x32\x88\xb8"
    write_pcap(PCAP_DIR / "goose_l2_nominal.pcap", [goose_eth + struct.pack(">HHHHII", 0x0001, 16, 0, 0, 10, i + 1) for i in range(10)])
    write_pcap(PCAP_DIR / "goose_l2_spoof_attack.pcap", [goose_eth + struct.pack(">HHHHII", 0x0001, 16, 0, 0, 2, 1) for _ in range(10)])

    itch_ok = b"A" + (b"\x00" * 19) + struct.pack(">I", 100) + (b"\x00" * 8) + struct.pack(">I", 1425000)
    itch_bad = b"A" + (b"\x00" * 19) + struct.pack(">I", 500000) + (b"\x00" * 8) + struct.pack(">I", 1000000)
    write_pcap(PCAP_DIR / "itch_nominal.pcap", [wrap_eth_ipv4_udp("10.240.60.50", "10.240.60.103", 40000, 9001, itch_ok) for _ in range(10)])
    write_pcap(PCAP_DIR / "itch_fat_finger_attack.pcap", [wrap_eth_ipv4_udp("10.240.60.99", "10.240.60.103", 40000, 9001, itch_bad) for _ in range(10)])
    print(f"[+] Generated 6 Telco/L2/FinTech PCAPs in {PCAP_DIR}")


if __name__ == "__main__":
    main()
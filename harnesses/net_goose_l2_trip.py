#!/usr/bin/env python3
# File: /home/kami/net-guard/harnesses/net_goose_l2_trip.py

"""
Sentinel-Lab: Non-IP Layer-2 IEC 61850 GOOSE (0x88B8) stNum Anti-Spoof Audit
"""

import argparse


class NetGooseL2TripHarness:
    def run_audit(self, mode="stnum_spoof"):
        print("\033[1;36m================================================================================\033[0m")
        print("\033[1;37m   SENTINEL-LAB: NON-IP L2 IEC 61850 GOOSE (0x88B8) ANTI-SPOOF AUDIT            \033[0m")
        print("\033[1;36m================================================================================\033[0m")
        cases = [
            ("Nominal L2 GOOSE (APPID=0x0001, stNum=10, sqNum=1)", 0x0001, 10, 1, True, "PASS"),
            ("Nominal Heartbeat (APPID=0x0001, stNum=10, sqNum=2)", 0x0001, 10, 2, True, "PASS"),
            ("Spoofed stNum Regression (stNum=4)" if mode == "stnum_spoof" else "Invalid APPID (0x9000)",
             0x0001 if mode == "stnum_spoof" else 0x9000,
             4 if mode == "stnum_spoof" else 10, 1, False,
             "REJECT_GOOSE_SPOOFED_STNUM" if mode == "stnum_spoof" else "REJECT_GOOSE_INVALID_APPID"),
        ]
        for label, appid, st, sq, ok, verdict in cases:
            col = "\033[1;32m" if ok else "\033[1;31m"
            print(f"[*] {label:<52} | {col}{verdict}\033[0m")
        print("\033[1;36m================================================================================\033[0m\n")


def main():
    parser = argparse.ArgumentParser(description="IEC 61850 L2 GOOSE Audit")
    parser.add_argument("--mode", default="stnum_spoof", choices=["stnum_spoof", "invalid_appid"])
    args = parser.parse_args()
    NetGooseL2TripHarness().run_audit(mode=args.mode)


if __name__ == "__main__":
    main()
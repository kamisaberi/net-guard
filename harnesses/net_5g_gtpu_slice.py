#!/usr/bin/env python3
# File: /home/kami/net-guard/harnesses/net_5g_gtpu_slice.py

"""
Sentinel-Lab: 3GPP 5G UPF GTP-U TEID & Slice-Hopping Isolation Audit
"""

import argparse


class Net5gGtpuSliceHarness:
    URLLC_TEIDS = range(0x100, 0x200)
    EMBB_TEIDS = range(0x200, 0x300)
    OT_PORTS = {502, 102, 2404}

    def evaluate_gtpu(self, teid: int, inner_dst_port: int):
        if teid in self.URLLC_TEIDS:
            return True, "PASS"
        if teid in self.EMBB_TEIDS:
            if inner_dst_port in self.OT_PORTS:
                return False, "REJECT_GTPU_SLICE_HOPPING"
            return True, "PASS"
        return False, "REJECT_GTPU_UNAUTHORIZED_TEID"

    def run_audit(self, mode="slice_hop"):
        print("\033[1;36m================================================================================\033[0m")
        print("\033[1;37m   SENTINEL-LAB: 5G UPF GTP-U SLICE-HOPPING & TEID ISOLATION AUDIT              \033[0m")
        print("\033[1;36m================================================================================\033[0m")
        cases = [
            ("URLLC Slice (TEID=0x100) -> Modbus :502", 0x0100, 502),
            ("eMBB Slice (TEID=0x200) -> HTTPS :443", 0x0200, 443),
            ("eMBB Slice Hopping -> Modbus :502" if mode == "slice_hop" else "Rogue TEID=0x9999",
             0x0200 if mode == "slice_hop" else 0x9999, 502),
        ]
        for label, teid, port in cases:
            ok, verdict = self.evaluate_gtpu(teid, port)
            col = "\033[1;32m" if ok else "\033[1;31m"
            print(f"[*] {label:<42} | {col}{verdict}\033[0m")
        print("\033[1;36m================================================================================\033[0m\n")


def main():
    parser = argparse.ArgumentParser(description="5G UPF GTP-U Slice Audit")
    parser.add_argument("--mode", default="slice_hop", choices=["slice_hop", "rogue_teid"])
    args = parser.parse_args()
    Net5gGtpuSliceHarness().run_audit(mode=args.mode)


if __name__ == "__main__":
    main()
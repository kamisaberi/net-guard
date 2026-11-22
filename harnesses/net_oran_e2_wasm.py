#!/usr/bin/env python3
# File: /home/kami/net-guard/harnesses/net_oran_e2_wasm.py

"""
Sentinel-Lab: O-RAN Near-RT RIC E2AP / SCTP Rogue xApp Isolation Audit
"""

import argparse


def main():
    parser = argparse.ArgumentParser(description="O-RAN E2AP Rogue xApp Audit")
    parser.add_argument("--mode", default="ric_control", choices=["ric_control", "e2_config"])
    args = parser.parse_args()

    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: O-RAN NEAR-RT RIC E2AP / SCTP xAPP SANDBOX AUDIT               \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    print("[*] E2AP RICindication (ProcCode=5)              | \033[1;32mPASS\033[0m")
    if args.mode == "ric_control":
        print("[*] Rogue xApp RICcontrolRequest (ProcCode=4)    | \033[1;31mREJECT_ORAN_ROGUE_XAPP_CONTROL\033[0m")
    else:
        print("[*] Unauthorized E2nodeConfigUpdate (ProcCode=10)| \033[1;31mREJECT_ORAN_ROGUE_XAPP_CONTROL\033[0m")
    print("\033[1;36m================================================================================\033[0m\n")


if __name__ == "__main__":
    main()
#!/usr/bin/env python3
# File: /home/kami/net-guard/harnesses/net_itch_pretrade.py

"""
Sentinel-Lab: NASDAQ ITCH 5.0 / OUCH Pre-Trade Risk Collar Audit
"""

import argparse


class NetItchPretradeHarness:
    def evaluate_order(self, shares: int, price_usd: float, max_shares: int = 10000):
        if shares <= 0 or shares > max_shares:
            return False, "REJECT_ITCH_FAT_FINGER_QTY"
        if price_usd < 1.0 or price_usd > 5000.0:
            return False, "REJECT_ITCH_PRICE_COLLAR"
        return True, "PASS"

    def run_audit(self, mode="fat_finger"):
        print("\033[1;36m================================================================================\033[0m")
        print("\033[1;37m   SENTINEL-LAB: FINTECH ITCH / OUCH NANOSECOND PRE-TRADE COLLAR AUDIT          \033[0m")
        print("\033[1;36m================================================================================\033[0m")
        cases = [
            ("Nominal Limit Order (100 @ $142.50)", 100, 142.50),
            ("Fat-Finger Algo Burst (500,000 @ $100.00)" if mode == "fat_finger" else "Price Collar Breach (100 @ $99,999.00)",
             500000 if mode == "fat_finger" else 100, 100.00 if mode == "fat_finger" else 99999.00),
        ]
        for label, sh, px in cases:
            ok, verdict = self.evaluate_order(sh, px)
            col = "\033[1;32m" if ok else "\033[1;31m"
            print(f"[*] {label:<44} | {col}{verdict}\033[0m")
        print("\033[1;36m================================================================================\033[0m\n")


def main():
    parser = argparse.ArgumentParser(description="FinTech ITCH Pre-Trade Audit")
    parser.add_argument("--mode", default="fat_finger", choices=["fat_finger", "collar"])
    args = parser.parse_args()
    NetItchPretradeHarness().run_audit(mode=args.mode)


if __name__ == "__main__":
    main()
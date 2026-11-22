# File: /home/kami/net-guard/apparatus_plugins/itch_orderbook.py

import math
import struct
import time


class ItchOrderbookPlugin:
    def init_state(self, engine):
        engine.domain_state["itch"] = {
            "symbol": "ARYO-ETF",
            "bid_usd": 142.48,
            "ask_usd": 142.52,
            "last_qty": 100,
            "collar_drops": 0,
            "gate_ns": 14.2,
            "status": "PRETRADE_COLLAR_ARMED",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        it = engine.domain_state["itch"]
        if is_attack:
            it["last_qty"] = 500000
            it["collar_drops"] += 1
            it["status"] = "FAT_FINGER_ORDER_DROPPED"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            it = engine.domain_state["itch"]
            if not engine.active_attack["active"]:
                mid = 142.50 + math.sin(elapsed * 1.1) * 0.35
                it["bid_usd"] = round(mid - 0.02, 2)
                it["ask_usd"] = round(mid + 0.02, 2)
                it["last_qty"] = 100 + int((elapsed * 10) % 400)
                it["status"] = "PRETRADE_COLLAR_ARMED"
            qty = it["last_qty"]
        pkt = b"A" + (b"\x00" * 19) + struct.pack(">I", qty) + (b"\x00" * 8) + struct.pack(">I", 1425000)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = b"A" + (b"\x00" * 19) + struct.pack(">I", 500000) + (b"\x00" * 8) + struct.pack(">I", 1000000)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        it = engine.domain_state["itch"]
        it["last_qty"] = 500000
        it["collar_drops"] += 1
        it["status"] = "FAT_FINGER_ORDER_DROPPED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"itch": dict(engine.domain_state["itch"])}


APPARATUS_TYPES = {"itch_orderbook": ItchOrderbookPlugin}
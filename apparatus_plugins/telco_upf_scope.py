# File: /home/kami/net-guard/apparatus_plugins/telco_upf_scope.py

import math
import struct
import time


class TelcoUpfScopePlugin:
    def init_state(self, engine):
        engine.domain_state["upf"] = {
            "urllc_gbps": 4.82,
            "embb_gbps": 18.4,
            "slice_hops_blocked": 0,
            "teid_active": "0x00000100 (URLLC)",
            "oran_ric_latency_us": 0.62,
            "status": "SLICE_ISOLATION_STRICT",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        u = engine.domain_state["upf"]
        if is_attack:
            u["slice_hops_blocked"] += 1
            u["teid_active"] = "0x00000200 (eMBB -> :502 BLOCKED)"
            u["oran_ric_latency_us"] = 0.81
            u["status"] = "SLICE_HOP_MITIGATED"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            if not engine.active_attack["active"]:
                u = engine.domain_state["upf"]
                u["urllc_gbps"] = round(4.82 + math.sin(elapsed * 1.4) * 0.18, 2)
                u["embb_gbps"] = round(18.4 + math.cos(elapsed * 0.9) * 0.65, 2)
                u["teid_active"] = "0x00000100 (URLLC)"
                u["oran_ric_latency_us"] = round(0.62 + abs(math.sin(elapsed)) * 0.08, 2)
                u["status"] = "SLICE_ISOLATION_STRICT"
        pkt = struct.pack(">BBHI", 0x30, 0xFF, 24, 0x00000100) + b"\x45\x00\x00\x18" + b"\x00" * 5 + b"\x06" + b"\x00" * 10 + struct.pack(">HH", 12345, 502)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = struct.pack(">BBHI", 0x30, 0xFF, 24, 0x00000200) + b"\x45\x00\x00\x18" + b"\x00" * 5 + b"\x06" + b"\x00" * 10 + struct.pack(">HH", 12345, 502)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        u = engine.domain_state["upf"]
        u["slice_hops_blocked"] += 1
        u["teid_active"] = "0x00000200 (eMBB -> :502 BLOCKED)"
        u["status"] = "SLICE_HOP_MITIGATED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"upf": dict(engine.domain_state["upf"])}


APPARATUS_TYPES = {"upf_slice_scope": TelcoUpfScopePlugin}
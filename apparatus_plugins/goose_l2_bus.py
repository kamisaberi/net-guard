# File: /home/kami/net-guard/apparatus_plugins/goose_l2_bus.py

import struct
import time


class GooseL2BusPlugin:
    def init_state(self, engine):
        engine.domain_state["goose"] = {
            "ethertype": "0x88B8 (NON-IP L2)",
            "appid": "0x0001",
            "st_num": 10,
            "sq_num": 1,
            "spoofs_blocked": 0,
            "pqc_suite": "X25519MLKEM768",
            "status": "L2_BUS_LOCKED",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        g = engine.domain_state["goose"]
        if is_attack:
            g["spoofs_blocked"] += 1
            g["status"] = "GOOSE_STNUM_SPOOF_DROPPED"
        else:
            g["sq_num"] = (g["sq_num"] % 65000) + 1

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            g = engine.domain_state["goose"]
            if not engine.active_attack["active"]:
                g["status"] = "L2_BUS_LOCKED"
            st, sq = g["st_num"], g["sq_num"]
        pkt = struct.pack(">HHHHII", 0x0001, 16, 0, 0, st, sq)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = struct.pack(">HHHHII", 0x0001, 16, 0, 0, 2, 1)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        g = engine.domain_state["goose"]
        g["spoofs_blocked"] += 1
        g["status"] = "GOOSE_STNUM_SPOOF_DROPPED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"goose": dict(engine.domain_state["goose"])}


APPARATUS_TYPES = {"goose_l2_bus": GooseL2BusPlugin}
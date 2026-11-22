// File: /home/kami/net-guard/web/net/net.js
// ==============================================================================
// ARYORITHM NET-GUARD UI PLUGIN:
//   1. 5G UPF GTP-U & O-RAN E2 Slice Scope (:18096)
//   2. Non-IP Layer-2 IEC 61850 GOOSE Bus Analyzer (:18097)
//   3. FinTech ITCH Pre-Trade Collar Order Book (:18098)
//   4. 5G Telco, Raw L2 & FinTech Operations C2 Tab
// ==============================================================================

(function () {
    if (!window.SentinelUI) return;

    const HOST = window.location.hostname || "127.0.0.1";
    const UPF_API = `http://${HOST}:18096`;
    const GOOSE_API = `http://${HOST}:18097`;
    const ITCH_API = `http://${HOST}:18098`;

    const isNetNode = (node) =>
        /5g|upf|goose|fintech|itch|telco/i.test(node?.site || "") ||
        /upf|goose|fintech/i.test(node?.hostname || "");

    function mountSimpleTelemetryCanvas(apiUrl, drawFn) {
        return function mount(container) {
            container.innerHTML = `<div style="position:relative; width:100%; height:100%; background:#05080C; border-radius:4px; overflow:hidden; aspect-ratio:1.25;"><canvas width="750" height="600" style="width:100%; height:100%; object-fit:cover; display:block;"></canvas></div>`;
            const canvas = container.querySelector("canvas");
            const state = { animId: null, alive: true, data: null, tick: 0 };

            const poll = async () => {
                try {
                    const r = await fetch(`${apiUrl}/telemetry`, { cache: "no-store" });
                    if (r.ok) state.data = await r.json();
                } catch (_) { }
            };
            poll();
            const timer = setInterval(poll, 220);

            function loop() {
                if (!state.alive) return;
                state.tick += 0.04;
                drawFn(canvas, state);
                state.animId = requestAnimationFrame(loop);
            }
            loop();
            state.cleanup = () => clearInterval(timer);
            return state;
        };
    }

    function unmountCanvas(handle) {
        if (!handle) return;
        handle.alive = false;
        if (handle.animId) cancelAnimationFrame(handle.animId);
        if (handle.cleanup) handle.cleanup();
    }

    // 1. 5G UPF GTP-U & O-RAN Slice Scope
    window.SentinelUI.registerApparatus({
        id: "net_5g_upf_scope",
        title: "5G UPF GTP-U & O-RAN E2 SLICE GOVERNOR",
        shortLabel: "📡 5G UPF SLICE",
        aspectRatio: 1.25,
        protocol: "GTP-U :2152 / O-RAN E2",
        matches: (node) => isNetNode(node) && /upf|5g/i.test(`${node?.site} ${node?.hostname}`),
        mount: mountSimpleTelemetryCanvas(UPF_API, (canvas, state) => {
            const ctx = canvas.getContext("2d");
            const w = canvas.width, h = canvas.height;
            const u = state.data?.upf || {};
            const alert = Boolean(state.data?.active_attack?.active);

            ctx.fillStyle = "#040812";
            ctx.fillRect(0, 0, w, h);

            ctx.strokeStyle = alert ? "#FF3366" : "#00E5FF";
            ctx.lineWidth = 2.2;
            ctx.beginPath();
            for (let x = 0; x < w; x++) {
                const y = (h * 0.42) + Math.sin(x * 0.02 + state.tick * 4.0) * (alert ? 48 : 26);
                if (x === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
            }
            ctx.stroke();

            ctx.strokeStyle = "#00FFA3";
            ctx.lineWidth = 2.0;
            ctx.beginPath();
            for (let x = 0; x < w; x++) {
                const y = (h * 0.68) + Math.cos(x * 0.03 - state.tick * 3.0) * 18;
                if (x === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
            }
            ctx.stroke();

            ctx.fillStyle = "rgba(7,11,20,0.88)";
            ctx.fillRect(0, 0, w, 28);
            ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace";
            ctx.fillStyle = alert ? "#FF3366" : "#00E5FF";
            ctx.fillText(alert ? `[!] eMBB SLICE-HOPPING BLOCKED | ${u.teid_active}` : `[+] 5G UPF STRICT ISOLATION | URLLC: ${u.urllc_gbps || 4.82} Gbps | eMBB: ${u.embb_gbps || 18.4} Gbps`, 12, 18);
            ctx.font = "9px monospace";
            ctx.fillStyle = "#94A3B8";
            ctx.fillText(`O-RAN RIC E2: ${u.oran_ric_latency_us || 0.62} µs | SLICE HOPS BLOCKED: ${u.slice_hops_blocked || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({
            apiBase: UPF_API,
            attacks: [
                { label: "[!] Inject 5G GTP-U Slice-Hopping PCAP", pcap: "gtpu_slice_hop_attack.pcap", attack_type: "slice_hop", mode: "overlay", duration_sec: 15 }
            ]
        })
    });

    // 2. Non-IP L2 IEC 61850 GOOSE Bus Analyzer
    window.SentinelUI.registerApparatus({
        id: "net_goose_l2_bus",
        title: "NON-IP LAYER-2 IEC 61850 GOOSE BUS (0x88B8)",
        shortLabel: "⚡ GOOSE L2 BUS",
        aspectRatio: 1.25,
        protocol: "AF_PACKET / 0x88B8",
        matches: (node) => isNetNode(node) && /goose|l2/i.test(`${node?.site} ${node?.hostname}`),
        mount: mountSimpleTelemetryCanvas(GOOSE_API, (canvas, state) => {
            const ctx = canvas.getContext("2d");
            const w = canvas.width, h = canvas.height;
            const g = state.data?.goose || {};
            const alert = Boolean(state.data?.active_attack?.active);

            ctx.fillStyle = "#040810";
            ctx.fillRect(0, 0, w, h);

            for (let row = 0; row < 4; row++) {
                const yBase = 110 + row * 105;
                ctx.strokeStyle = alert && row === 1 ? "#FF3366" : "#00FFA3";
                ctx.lineWidth = 2;
                ctx.beginPath();
                for (let x = 20; x < w - 20; x += 18) {
                    const pulse = ((Math.floor(x / 18 + state.tick * 4 + row) % 5) === 0) ? -28 : 0;
                    if (x === 20) ctx.moveTo(x, yBase + pulse); else ctx.lineTo(x, yBase + pulse);
                }
                ctx.stroke();
            }

            ctx.fillStyle = "rgba(7,11,20,0.88)";
            ctx.fillRect(0, 0, w, 28);
            ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace";
            ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillText(alert ? `[!] NON-IP L2 GOOSE stNum SPOOF DROPPED | APPID ${g.appid || "0x0001"}` : `[+] L2 GOOSE BUS (0x88B8) | stNum=${g.st_num || 10} sqNum=${g.sq_num || 1} | PQC=${g.pqc_suite || "ML-KEM"}`, 12, 18);
            ctx.font = "9px monospace";
            ctx.fillStyle = "#94A3B8";
            ctx.fillText(`AF_PACKET RAW L2 CAPTURE | SPOOFS BLOCKED: ${g.spoofs_blocked || 0} | SLA < 120 ns`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({
            apiBase: GOOSE_API,
            attacks: [
                { label: "[!] Inject L2 GOOSE stNum Spoof PCAP", pcap: "goose_l2_spoof_attack.pcap", attack_type: "goose_spoof", mode: "overlay", duration_sec: 15 }
            ]
        })
    });

    // 3. FinTech ITCH Pre-Trade Order Book Firewall
    window.SentinelUI.registerApparatus({
        id: "net_itch_orderbook",
        title: "FINTECH ITCH 5.0 PRE-TRADE RISK COLLAR",
        shortLabel: "💹 ITCH COLLAR",
        aspectRatio: 1.25,
        protocol: "ITCH 5.0 / OUCH :9001",
        matches: (node) => isNetNode(node) && /fintech|itch|exchange/i.test(`${node?.site} ${node?.hostname}`),
        mount: mountSimpleTelemetryCanvas(ITCH_API, (canvas, state) => {
            const ctx = canvas.getContext("2d");
            const w = canvas.width, h = canvas.height;
            const it = state.data?.itch || {};
            const alert = Boolean(state.data?.active_attack?.active);

            ctx.fillStyle = "#040711";
            ctx.fillRect(0, 0, w, h);

            for (let i = 0; i < 8; i++) {
                const bw = 120 + ((Math.sin(state.tick + i) + 1) * 110);
                ctx.fillStyle = "rgba(0, 255, 163, 0.28)";
                ctx.fillRect(40, 80 + i * 54, bw, 32);
                ctx.fillStyle = alert && i === 0 ? "rgba(255, 51, 102, 0.65)" : "rgba(0, 229, 255, 0.28)";
                ctx.fillRect(w - 40 - bw, 80 + i * 54, bw, 32);
            }

            ctx.fillStyle = "rgba(7,11,20,0.88)";
            ctx.fillRect(0, 0, w, 28);
            ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace";
            ctx.fillStyle = alert ? "#FF3366" : "#00E5FF";
            ctx.fillText(alert ? `[!] FAT-FINGER ORDER BLOCKED (${it.last_qty || 500000} SHARES)` : `[+] ITCH PRE-TRADE GATE | ${it.symbol || "ARYO-ETF"} BID $${it.bid_usd || 142.48} / ASK $${it.ask_usd || 142.52}`, 12, 18);
            ctx.font = "9px monospace";
            ctx.fillStyle = "#94A3B8";
            ctx.fillText(`GATE LATENCY: ${it.gate_ns || 14.2} ns | COLLAR DROPS: ${it.collar_drops || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({
            apiBase: ITCH_API,
            attacks: [
                { label: "[!] Inject ITCH Fat-Finger Burst PCAP", pcap: "itch_fat_finger_attack.pcap", attack_type: "fat_finger", mode: "overlay", duration_sec: 15 }
            ]
        })
    });

    // 4. Telco 5G, Raw L2 & FinTech C2 Tab
    let c2Timer = null;
    function renderNetC2(container) {
        if (c2Timer) clearInterval(c2Timer);
        container.innerHTML = `
            <div style="display:flex; flex-direction:column; gap:14px;">
                <div class="net-c2-bar">
                    <div style="display:flex; align-items:center; gap:10px;">
                        <span style="font-size:12px; font-weight:800; color:#00E5FF; letter-spacing:0.08em;">
                            NET-GUARD: 5G UPF, NON-IP L2 GOOSE &amp; FINTECH C2
                        </span>
                        <span class="badge badge-green" style="font-size:9px;">AF_PACKET + PQC ML-KEM</span>
                    </div>
                    <div style="font-family:monospace; font-size:11px; color:#94A3B8;">
                        STREAMS: :18096 (5G UPF) &bull; :18097 (L2 GOOSE) &bull; :18098 (ITCH)
                    </div>
                </div>
                <div class="net-c2-grid">
                    <div id="net-card-upf" class="net-c2-card">
                        <div style="color:#00E5FF; font-weight:800; font-size:11.5px;">📡 5G UPF + O-RAN (10.240.60.101)</div>
                        <div id="net-upf-kpis">Connecting to :18096...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="net-c2-btn danger" data-net-atk="upf">INJECT SLICE HOP</button>
                            <button class="net-c2-btn" data-net-stop="upf">RESTORE</button>
                        </div>
                    </div>
                    <div id="net-card-goose" class="net-c2-card">
                        <div style="color:#00FFA3; font-weight:800; font-size:11.5px;">⚡ L2 GOOSE + PQC (10.240.60.102)</div>
                        <div id="net-goose-kpis">Connecting to :18097...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="net-c2-btn danger" data-net-atk="goose">INJECT GOOSE SPOOF</button>
                            <button class="net-c2-btn" data-net-stop="goose">RESTORE</button>
                        </div>
                    </div>
                    <div id="net-card-itch" class="net-c2-card">
                        <div style="color:#FFB800; font-weight:800; font-size:11.5px;">💹 FINTECH ITCH (10.240.60.103)</div>
                        <div id="net-itch-kpis">Connecting to :18098...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="net-c2-btn danger" data-net-atk="itch">INJECT FAT-FINGER</button>
                            <button class="net-c2-btn" data-net-stop="itch">RESTORE</button>
                        </div>
                    </div>
                </div>
            </div>
        `;

        const map = {
            upf: { url: UPF_API, body: { attack_type: "slice_hop", pcap: "gtpu_slice_hop_attack.pcap", mode: "overlay", duration_sec: 15 } },
            goose: { url: GOOSE_API, body: { attack_type: "goose_spoof", pcap: "goose_l2_spoof_attack.pcap", mode: "overlay", duration_sec: 15 } },
            itch: { url: ITCH_API, body: { attack_type: "fat_finger", pcap: "itch_fat_finger_attack.pcap", mode: "overlay", duration_sec: 15 } }
        };

        container.querySelectorAll("[data-net-atk]").forEach(b => b.addEventListener("click", () => {
            const c = map[b.getAttribute("data-net-atk")];
            if (c) fetch(`${c.url}/attack/inject`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(c.body) }).catch(() => {});
        }));
        container.querySelectorAll("[data-net-stop]").forEach(b => b.addEventListener("click", () => {
            const c = map[b.getAttribute("data-net-stop")];
            if (c) fetch(`${c.url}/attack/stop`, { method: "POST" }).catch(() => {});
        }));

        async function refresh() {
            const u = await fetch(`${UPF_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            const g = await fetch(`${GOOSE_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            const i = await fetch(`${ITCH_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            if (u?.upf) {
                const al = Boolean(u.active_attack?.active);
                document.getElementById("net-card-upf")?.classList.toggle("alert", al);
                document.getElementById("net-upf-kpis").innerHTML = `<div class="net-c2-kpi"><span>URLLC / eMBB</span><b>${u.upf.urllc_gbps}/${u.upf.embb_gbps} Gbps</b></div><div class="net-c2-kpi"><span>Slice Status</span><b style="color:${al ? "#FF3366" : "#00FFA3"};">${u.upf.status}</b></div>`;
            }
            if (g?.goose) {
                const al = Boolean(g.active_attack?.active);
                document.getElementById("net-card-goose")?.classList.toggle("alert", al);
                document.getElementById("net-goose-kpis").innerHTML = `<div class="net-c2-kpi"><span>L2 EtherType</span><b>${g.goose.ethertype}</b></div><div class="net-c2-kpi"><span>stNum / sqNum</span><b>${g.goose.st_num} / ${g.goose.sq_num}</b></div>`;
            }
            if (i?.itch) {
                const al = Boolean(i.active_attack?.active);
                document.getElementById("net-card-itch")?.classList.toggle("alert", al);
                document.getElementById("net-itch-kpis").innerHTML = `<div class="net-c2-kpi"><span>ARYO-ETF</span><b>$${i.itch.bid_usd} / $${i.itch.ask_usd}</b></div><div class="net-c2-kpi"><span>Collar Drops</span><b style="color:${al ? "#FF3366" : "#00FFA3"};">${i.itch.collar_drops}</b></div>`;
            }
        }
        refresh();
        c2Timer = setInterval(refresh, 500);
    }

    window.SentinelUI.registerTab({
        id: "tab-net-c2",
        label: "5G & FinTech Wire C2",
        icon: "🌐",
        group: "Operations",
        renderView: renderNetC2
    });
})();
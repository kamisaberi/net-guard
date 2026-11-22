# File: /home/kami/net-guard/Makefile
# ==============================================================================
# ARYORITHM TECHNOLOGIES B.V. — HIGH-SPEED WIRE, 5G, FINTECH & PQC SYSTEMS
# Project: net-guard (5G UPF GTP-U, O-RAN E2, IEC 61850 L2, FinTech ITCH, PQC)
# Invariant: Sub-120 ns Deterministic Tier-1 Gate | Zero Dynamic Alloc
# ==============================================================================

SHELL := /bin/bash
.DEFAULT_GOAL := help

PROJECT_DIR   := $(shell pwd)
BUILD_DIR     := $(PROJECT_DIR)/build
SCENARIOS_DIR := $(PROJECT_DIR)/scenarios/telco_5g_fintech_grid
MATRIX_DIR    := /home/kami/sentinel-matrix
NEXUS_DIR     := /home/kami/sentinel-nexus
ETC_HARNESSES := /etc/sentinel/lab/harnesses
OPT_HARNESSES := /opt/sentinel-stack/harnesses
JOBS          ?= $(shell nproc)

CYAN   := \033[1;36m
GREEN  := \033[1;32m
YELLOW := \033[1;33m
RED    := \033[1;31m
WHITE  := \033[1;37m
RESET  := \033[0m

.PHONY: help
help:
	@echo -e "$(CYAN)================================================================================$(RESET)"
	@echo -e "$(WHITE)  NET-GUARD: 5G UPF, O-RAN, RAW L2 GOOSE, FINTECH ITCH & PQC DEFENSE ENGINE$(RESET)"
	@echo -e "$(CYAN)================================================================================$(RESET)"
	@echo -e "  $(GREEN)make init$(RESET)         Hard copy all net research harnesses to /etc and /opt"
	@echo -e "  $(GREEN)make build$(RESET)        Compile release targets (-O3 -march=native)"
	@echo -e "  $(GREEN)make test$(RESET)         Run all unit, ABI & Tier-1/Tier-2 pipeline tests"
	@echo -e "  $(GREEN)make bench$(RESET)        Execute hardware latency benchmark (< 120 ns SLA)"
	@echo -e "  $(GREEN)make install$(RESET)      Install libraries, CLI & plugin system-wide"
	@echo -e "  $(GREEN)make sync-all$(RESET)     Full synchronization (install + sync-matrix + sync-web)"
	@echo -e "  $(GREEN)make telco-up$(RESET)     Deploy telco_5g_fintech_grid simulation mesh"
	@echo -e "  $(GREEN)make telco-down$(RESET)   Teardown telco_5g_fintech_grid and release subnets"
	@echo -e "  $(GREEN)make telco-status$(RESET) Display running containers and ports"
	@echo -e "  $(GREEN)make telco-logs$(RESET)   Stream live logs from 5G UPF Edge Sentry"
	@echo -e "$(CYAN)================================================================================$(RESET)"

.PHONY: init
init:
	@sudo mkdir -p $(ETC_HARNESSES) $(OPT_HARNESSES)
	@chmod +x harnesses/*.py 2>/dev/null || true
	@for f in harnesses/*.py; do \
		if [ -f "$$f" ]; then \
			base=$$(basename $$f); \
			hyphen_base=$$(echo $$base | tr '_' '-'); \
			sudo rm -f $(ETC_HARNESSES)/$$base $(ETC_HARNESSES)/$$hyphen_base; \
			sudo cp -f $$f $(ETC_HARNESSES)/$$base; \
			sudo cp -f $$f $(ETC_HARNESSES)/$$hyphen_base; \
			sudo rm -f $(OPT_HARNESSES)/$$base $(OPT_HARNESSES)/$$hyphen_base; \
			sudo cp -f $$f $(OPT_HARNESSES)/$$base; \
			sudo cp -f $$f $(OPT_HARNESSES)/$$hyphen_base; \
		fi \
	done
	@sudo chmod 755 $(ETC_HARNESSES)/*.py $(OPT_HARNESSES)/*.py 2>/dev/null || true
	@echo -e "$(GREEN)[+] Net-Guard harnesses installed to $(ETC_HARNESSES) and $(OPT_HARNESSES).$(RESET)"

.PHONY: all build debug
all: build

build:
	@echo -e "$(CYAN)[+] Configuring & Building Net-Guard Release Targets (Jobs: $(JOBS))...$(RESET)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)
	@echo -e "$(GREEN)[+] Build Complete: libnet_guard.so, net_guard_dissector_plugin.so, aryo-net.$(RESET)"

debug:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)

.PHONY: test bench
test: build
	@echo -e "$(CYAN)[+] Running net-guard Test Suite...$(RESET)"
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo -e "$(GREEN)[+] All Net-Guard Unit, ABI & Pipeline Tests Passed (100%).$(RESET)"

bench: build
	@$(BUILD_DIR)/bench_latency

.PHONY: install sync-matrix sync-web sync-all
install: build init
	@sudo $(MAKE) -C $(BUILD_DIR) install
	@sudo ldconfig
	@echo -e "$(GREEN)[+] Installed libnet_guard.so, net_guard_dissector_plugin.so, aryo-net, net-guard-ctl.$(RESET)"

sync-matrix: build
	@mkdir -p $(MATRIX_DIR)/shared/lib
	@cp -f $(BUILD_DIR)/libnet_guard.so $(MATRIX_DIR)/shared/lib/
	@echo -e "$(GREEN)[+] Synced libnet_guard.so -> $(MATRIX_DIR)/shared/lib/$(RESET)"

sync-web:
	@mkdir -p $(MATRIX_DIR)/web/plugins
	@if [ -d "$(NEXUS_DIR)/web" ]; then \
		cp -r $(NEXUS_DIR)/web/* $(MATRIX_DIR)/web/ 2>/dev/null || true; \
		mkdir -p $(MATRIX_DIR)/web/plugins; \
	fi

sync-all: install sync-matrix sync-web
	@echo -e "$(GREEN)[+] Full Net-Guard Synchronization Complete.$(RESET)"

.PHONY: telco-up telco-down telco-status telco-logs
telco-up: sync-all
	@aryo net telco-up
	@echo -e "$(GREEN)[+] Telco 5G & FinTech Grid Live: http://localhost:9443/$(RESET)"

telco-down:
	@aryo net telco-down || true
	@docker network prune -f >/dev/null 2>&1 || true

telco-status:
	@docker ps --filter "name=matrix-" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

telco-logs:
	@docker logs -f matrix-edge-upf-01

.PHONY: clean distclean
clean:
	@rm -rf $(BUILD_DIR)

distclean: clean
	@$(MAKE) telco-down >/dev/null 2>&1 || true
	@rm -rf $(MATRIX_DIR)/.matrix_cache/docker-compose.telco_5g_fintech_grid.yml
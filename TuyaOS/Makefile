# -----------------------------------------------------------------------------
# Packaged TuyaOS: minimal Makefile for application Kconfig only.
# Use from this directory: make app_menuconfig / app_config / app_config_choice
# Full IoT SDK builds use the repository root Makefile instead.
# -----------------------------------------------------------------------------
export HAS_PYTHON3 := $(shell which python3)
APP_NAME ?= tuyaos_demo_wukong_ai
include scripts/mk/config.mk

# ---------------------------------------------------------------------------
# 应用层 Kconfig 自动化（不修改公共 config.mk）
#
# 扫描 application_components/ 与 application_drivers/ 下所有子目录的
# IoTOSconfig，生成 apps/$(APP_NAME)/build/auto_components_kconfig 片段。
# APPconfig 里用 `orsource "auto_components_kconfig"` 引用即可自动 rsource。
#
#   make gen_app_kconfig       APP_NAME=xxx   # 仅生成片段
#   make app_menuconfig_auto   APP_NAME=xxx   # 生成片段 + 启动 menuconfig
# ---------------------------------------------------------------------------
gen_app_kconfig:
	@if [ -z "$(APP_NAME)" ]; then echo "APP_NAME is required"; exit 1; fi
	@bash ./scripts/gen_app_components_kconfig.sh ./apps/$(APP_NAME)/build

app_menuconfig_auto: gen_app_kconfig
	@if [ "$(HAS_PYTHON3)" = "" ]; then echo "has no python3"; exit 1; fi
	@if [ -z "$(APP_NAME)" ]; then echo "APP_NAME is required"; exit 1; fi
	@if [ ! -f ./apps/$(APP_NAME)/build/tuya_app.config ]; then \
	    echo "tuya_app.config not found, generating default config via olddefconfig..."; \
	    KCONFIG_CONFIG=./apps/$(APP_NAME)/build/tuya_app.config \
	        python3 ./scripts/kconfiglib/olddefconfig.py ./apps/$(APP_NAME)/build/APPconfig; \
	fi
	@$(MAKE) app_menuconfig APP_NAME=$(APP_NAME)
	@$(MAKE) app_config    APP_NAME=$(APP_NAME)

.PHONY: gen_app_kconfig app_menuconfig_auto

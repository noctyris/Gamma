# You can override those settings on the command line

PLATFORM ?= device
DEBUG ?= 0

HOME_DISPLAY_EXTERNALS ?= 1
EPSILON_VERSION ?= 15.5.0
OMEGA_VERSION ?= 2.0.2
OMEGA_VERSION_SHORT ?= 2.0 # 3 chars max, required for compatibility
UPSILON_VERSION ?= 1.1.2
# Gamma's version is derived from the number of commits since the fork point,
# so it increments automatically without a manual version file to edit.
GAMMA_VERSION_MAJOR ?= 1
GAMMA_VERSION_MINOR ?= 0
GAMMA_VERSION_BASE_COMMIT ?= e8ceb79bd
GAMMA_VERSION_PATCH := $(shell git rev-list --count $(GAMMA_VERSION_BASE_COMMIT)..HEAD 2>/dev/null || echo 0)
GAMMA_VERSION ?= $(GAMMA_VERSION_MAJOR).$(GAMMA_VERSION_MINOR).$(GAMMA_VERSION_PATCH)
# OMEGA_USERNAME ?= N/A
OMEGA_STATE ?= dev
EPSILON_APPS ?= calculation graph finance code statistics probability solver atomic sequence regression reader settings external
SUBMODULES_APPS = atomic
EPSILON_I18N ?= en fr nl pt it de es hu
EPSILON_COUNTRIES ?= WW CA DE ES FR GB IT NL PT US
EPSILON_GETOPT ?= 0
ESCHER_LOG_EVENTS_BINARY ?= 0
THEME_NAME ?= upsilon_light
THEME_REPO ?= local
INCLUDE_ULAB ?= 1

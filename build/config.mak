# You can override those settings on the command line

PLATFORM ?= device
DEBUG ?= 0

HOME_DISPLAY_EXTERNALS ?= 1
EPSILON_VERSION ?= 15.5.0
OMEGA_VERSION ?= 2.0.2
OMEGA_VERSION_SHORT ?= 2.0 # 3 chars max, required for compatibility
UPSILON_VERSION ?= 1.1.2

# Gamma version, derived from git history so it increments without a manual
# version file:
#   - every regular commit increments the patch number;
#   - every integration merge increments the minor number and resets the patch.
# Override GAMMA_VERSION_MAJOR or GAMMA_VERSION on the command line for a
# release. The base commit is the state of gamma-dev when this scheme started.
GAMMA_VERSION_MAJOR ?= 1
GAMMA_VERSION_BASE_COMMIT ?= 79de19596
GAMMA_VERSION_BASE := $(shell if git merge-base --is-ancestor $(GAMMA_VERSION_BASE_COMMIT) HEAD 2>/dev/null; then echo $(GAMMA_VERSION_BASE_COMMIT); else echo HEAD; fi)
GAMMA_VERSION_LAST_MERGE := $(shell git rev-list --first-parent --merges -n 1 $(GAMMA_VERSION_BASE)..HEAD 2>/dev/null)
GAMMA_VERSION_MINOR := $(shell git rev-list --first-parent --merges --count $(GAMMA_VERSION_BASE)..HEAD 2>/dev/null || echo 0)
GAMMA_VERSION_PATCH := $(shell if [ -n "$(GAMMA_VERSION_LAST_MERGE)" ]; then git rev-list --count --no-merges $(GAMMA_VERSION_LAST_MERGE)..HEAD 2>/dev/null || echo 0; else git rev-list --count --no-merges $(GAMMA_VERSION_BASE)..HEAD 2>/dev/null || echo 0; fi)
GAMMA_VERSION ?= $(GAMMA_VERSION_MAJOR).$(GAMMA_VERSION_MINOR).$(GAMMA_VERSION_PATCH)

# GAMMA_USERNAME is the preferred spelling. OMEGA_USERNAME remains accepted
# for compatibility with older build scripts; build/defaults.mak maps it.
GAMMA_USERNAME ?= $(OMEGA_USERNAME)
OMEGA_STATE ?= dev
EPSILON_APPS ?= calculation graph rpn code statistics probability solver atomic sequence regression reader settings external
SUBMODULES_APPS = atomic rpn
EPSILON_I18N ?= en fr nl pt it de es hu
EPSILON_COUNTRIES ?= WW CA DE ES FR GB IT NL PT US
EPSILON_GETOPT ?= 0
ESCHER_LOG_EVENTS_BINARY ?= 0
THEME_NAME ?= gamma_dark
THEME_REPO ?= local
INCLUDE_ULAB ?= 1

# Gamma

Gamma is a community operating system for NumWorks calculators. It continues the
public **Epsilon 15.5.0 → Omega → Upsilon → Gamma** lineage. This document
describes the code layout, the supported build and flashing paths, the
compatibility policy, and the rules for tracking upstream work.

## Status and scope

- Base source: **Epsilon 15.5.0**, published under CC BY-NC-SA 4.0.
- Inherited projects: **Omega 2.0.2** and **Upsilon 1.1.2**, also under
  CC BY-NC-SA 4.0.
- Current product version: derived from git history as `GAMMA_VERSION`; run
  `make info` to print it.
- Upstream Epsilon 16.0.0 and later are **not** imported. Those versions are
  published under “All rights reserved”. See [NOTICE.md](../NOTICE.md).

The internal executable and artifact names (`epsilon`, `epsilon.dfu`,
`epsilon.js`) are kept for compatibility with the existing build system and
bootloader tooling. They are not a claim of affiliation with NumWorks.

## Architecture

Gamma keeps the five historical bricks of Epsilon and adds the Omega/Upsilon
applications and tools.

| Layer | Directory | Role |
| --- | --- | --- |
| Ion | `ion/` | Hardware and simulator abstraction: display, keyboard, USB, flash, battery, timing. |
| Kandinsky | `kandinsky/` | Low-level drawing, colours, fonts and images. |
| Escher | `escher/` | GUI toolkit: views, buttons, editors, tab bars and event handling. |
| Poincaré | `poincare/` | Expression parser, layout, simplification and numeric evaluation. |
| Applications | `apps/` | Calculation, graph, code, statistics, solver, reader, RPN, settings and external apps. |
| Python | `python/` | MicroPython port and Gamma extensions (ulab, `os.uname`, `getlogin`). |
| Bootloader | `bootloader/` | Slot selection, recovery and installation support. |
| Build system | `build/` | Platform, target, toolchain and theme selection. |

### Device and simulator split

The same application code is compiled against either the device implementation
of Ion or one of the simulator backends:

- `PLATFORM=device` builds for the calculator. `MODEL` selects the hardware
  variant (`n0100`, `n0110` or `bootloader`).
- `PLATFORM=simulator` builds a desktop or hosted target. `TARGET` selects
  `linux`, `windows`, `macos`, `android`, `ios`, `web`, `3ds`, `fxcg` or
  `nspire`.

Themes are compiled into the firmware and the simulator. The default is
`gamma_dark`; `gamma_light` is the matching light theme. The inherited
`omega_dark`, `omega_light`, `upsilon_light`, `epsilon_dark` and
`epsilon_light` themes remain available.

## Build

### Requirements

Linux (Debian/Ubuntu):

```bash
sudo apt-get install build-essential git imagemagick libx11-dev libxext-dev \
  libfreetype6-dev libpng-dev libjpeg-dev pkg-config \
  gcc-arm-none-eabi binutils-arm-none-eabi
```

macOS with Homebrew:

```bash
brew install freetype imagemagick libpng libusb pkg-config python3
python3 -m venv .venv
.venv/bin/pip3 install setuptools lz4 pypng stringcase
```

For the web target, install and activate emsdk 4.0.22. Android, iOS, macOS and
3DS builds additionally need their platform SDKs; they are optional.

### Simulator and tests

```bash
make PLATFORM=simulator TARGET=linux -j"$(nproc)"
make PLATFORM=simulator TARGET=linux test -j"$(nproc)"
./output/release/simulator/linux/test.bin --headless
```

### Theme

```bash
make PLATFORM=simulator TARGET=linux THEME_NAME=gamma_light -j"$(nproc)"
```

List the local themes with:

```bash
python3 themes/themes_manager.py --list
```

### Device

n0110 with the Gamma/Omega/Upsilon bootloader:

```bash
make MODEL=bootloader GAMMA_USERNAME="Your name" -j"$(nproc)"
make epsilon.A_flash
```

Legacy n0110 build without slot protection:

```bash
make MODEL=n0110 GAMMA_USERNAME="Your name" -j"$(nproc)"
make MODEL=n0110 epsilon_flash
```

n0100 (legacy, single language):

```bash
make MODEL=n0100 EPSILON_I18N=en GAMMA_USERNAME="Your name" -j"$(nproc)"
make MODEL=n0100 epsilon_flash
```

## Flashing and recovery

Flashing a custom firmware can erase calculator data. Save Python scripts and
any important state first, and only flash an artifact you built or verified.

- **Bootloader path:** install the Gamma/Omega/Upsilon bootloader, then flash a
  userland into slot A or B. The other slot is preserved.
- **Legacy n0110 path:** `epsilon_flash` writes the userland directly. It has no
  A/B protection; a bad image can require a full recovery.
- **n0100:** use the n0100 build only. The model has 1 MB of flash and a reduced
  feature set.
- **Recovery:** the bootloader recovery menu can reinstall a valid slot. Keep a
  known-good DFU image available before experimenting.

Do not use a firmware image from an untrusted source: firmware has full access
to the calculator.

## Compatibility matrix

| Target | Source relationship | Support |
| --- | --- | --- |
| Epsilon 15.5.0 | Base of Gamma; CC BY-NC-SA 4.0 | Supported source baseline. |
| Epsilon 16.0.0 and later | Not imported; “All rights reserved” | Not supported. Behaviour may be reimplemented cleanly, without copying code. |
| Epsilon 23.2.3 | Later official release; not imported | No source or ABI compatibility. Gamma is not a drop-in replacement for this firmware. |
| n0110 with Gamma bootloader | Native target | Supported: A/B slots, recovery, external apps. |
| n0110 legacy flash | Native target | Buildable, deprecated: no slot protection. |
| n0100 | Reduced legacy target | Buildable with one language, no Reader, Atomic, ulab or external apps. |
| Linux simulator | Native host target | Supported and covered by tests. |
| Web simulator | Emscripten target | Supported build; browser testing is manual. |
| Android, iOS, macOS, Windows, 3DS, fxcg, nspire | Hosted ports | Buildable when the platform SDK is present; less tested than Linux. |

The matrix describes source and installation support, not compatibility with
NumWorks cloud services, exam-mode certification, or official firmware update
tools. Gamma is an independent community project.

## EADK and NWA

Gamma keeps **NWA** as its external-app container. NWA is already integrated in
`apps/external/`, including the archive format, launch path, exam-mode blocking
and the external-app UI. The EADK syscall entry points in `ion/` are kept as a
small compatibility layer for apps that use them.

Decision for the current series:

- do not add a second, parallel EADK application launcher;
- keep the existing EADK SVC subset working for NWA apps;
- document NWA as the supported external-app path;
- revisit EADK only if a maintained, tested SDK and installer are available.

This avoids maintaining two app formats and two installation flows while
preserving compatibility with existing external apps.

## n0100 policy

The n0100 remains buildable as a **legacy target**. It receives no new
applications or features that increase flash usage. Reader, Atomic, ulab and
external apps stay disabled; Atomic is removed because the full app set overflows
the 1 MB flash. The model is increasingly rare and its flash is the limiting
factor. A future major release may remove the target entirely; until then,
changes must keep `make MODEL=n0100` buildable.

## Upstream policy

Gamma is a downstream fork of Epsilon 15.5.0, not a tracking branch of current
NumWorks releases.

1. Do not copy code, comments, assets or tests from Epsilon 16.0.0 or later
   without written permission from NumWorks.
2. If a newer behaviour is needed, reimplement it from the observable behaviour
   or public documentation and record the reference in the commit message.
3. Keep license and attribution notices intact when moving inherited code.
4. Use a temporary `upstream-sync` worktree for a no-commit test merge. The goal
   is a conflict report by module, not an import. The 16.0.0 report is in
   [UPSTREAM-16.0.0.md](./UPSTREAM-16.0.0.md).
5. Any upstream-derived change must pass the simulator build and the full test
   suite before review.

## Useful commands

```bash
make info
make help
make PLATFORM=simulator TARGET=linux test -j"$(nproc)"
make MODEL=n0110 epsilon.dfu -j"$(nproc)"
make PLATFORM=simulator TARGET=web htmlpack.zip -j"$(nproc)"
```

See [README.md](../README.md) for the high-level project overview and
[NOTICE.md](../NOTICE.md) for licensing and trademarks.

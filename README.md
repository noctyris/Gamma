<p align="center">
  <strong>Gamma</strong><br/>
  A community operating system for NumWorks calculators.
</p>

<p align="center">
  <a href="https://github.com/noctyris/Gamma/issues"><img alt="Issues" src="https://img.shields.io/github/issues/noctyris/Gamma?style=for-the-badge" /></a>
  <a href="https://github.com/noctyris/Gamma/commits/gamma-dev"><img alt="Last commit" src="https://img.shields.io/github/last-commit/noctyris/Gamma/gamma-dev?style=for-the-badge" /></a>
  <a href="./LICENSE.md"><img alt="License: CC BY-NC-SA 4.0" src="https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-525252.svg?labelColor=292929&logo=creative%20commons&style=for-the-badge" /></a>
</p>

> 🇫🇷 Vous préférez le français ? Lisez le [README français](./README.fr.md).

## About

Gamma is a community operating system for NumWorks calculators. It continues the
lineage **Epsilon 15.5.0 → Omega → Upsilon → Gamma** and keeps the features built
by the previous projects while adding its own.

Gamma is based on the public **Epsilon 15.5.0** sources. Epsilon 16.0.0 and later
are published by NumWorks without the CC BY-NC-SA license (the README states
“All rights reserved”), so Gamma does **not** import code from those versions.
See [NOTICE.md](./NOTICE.md).

## Features

- Gamma theme and theme engine, with light and dark themes
- Wallpapers and external applications (NWA)
- RPN and Atomic applications
- Reader application
- Python extensions: ulab, operator overloading, `os.uname`, `getlogin`
- Accessibility options: inverted colors, magnifier, RGB gamma correction
- Finance application (TVM)
- Graph improvements: implicit plots, inequalities, function types
- Eight languages: English, French, Dutch, Portuguese, Italian, German, Spanish, Hungarian
- NumWorks n0100 and n0110 support, including bootloader slots A/B/Khi

## Version

`GAMMA_VERSION` is derived from git history, so it increments without a manual
version file:

- a regular commit increments the patch number (`1.0.0` → `1.0.1`);
- an integration merge increments the minor number and resets the patch
  (`1.0.1` → `1.1.0`);
- the major number is set manually with `GAMMA_VERSION_MAJOR` or a full
  `GAMMA_VERSION` override for a release.

The version is shown in the bootloader's About menu and next to the Gamma slots.
`make info` prints the current value.

## Build

### Requirements

Gamma uses the same toolchain as Epsilon 15.5.0. On Debian or Ubuntu:

```bash
sudo apt-get install build-essential git imagemagick libx11-dev libxext-dev \
  libfreetype6-dev libpng-dev libjpeg-dev pkg-config \
  gcc-arm-none-eabi binutils-arm-none-eabi
```

On macOS with Homebrew:

```bash
brew install freetype imagemagick libpng libusb pkg-config python3
python3 -m venv .venv
.venv/bin/pip3 install setuptools lz4 pypng stringcase
```

On Windows, use MSYS2 or WSL 2. The ARM cross-compiler is only needed for device
builds.

### Clone

```bash
git clone --recursive https://github.com/noctyris/Gamma.git
cd Gamma
git checkout gamma-dev
```

### Simulator

```bash
make PLATFORM=simulator TARGET=linux -j$(nproc)
```

The simulator is written to `output/release/simulator/linux/`. To build and run
the tests:

```bash
make PLATFORM=simulator TARGET=linux test -j$(nproc)
./output/release/simulator/linux/test.bin --headless
```

### Web simulator

Install emsdk 4.0.22, activate it, then:

```bash
make PLATFORM=simulator TARGET=web htmlpack.zip -j$(nproc)
```

The archive is written to `output/release/simulator/web/htmlpack.zip`.

### NumWorks n0100

```bash
make MODEL=n0100 EPSILON_I18N=en GAMMA_USERNAME="Your name" -j$(nproc)
make MODEL=n0100 epsilon_flash
```

Only one language is supported on n0100; the build defaults to English and
removes the Reader and ulab.

### NumWorks n0110

With the Gamma/Omega/Upsilon bootloader:

```bash
make MODEL=bootloader GAMMA_USERNAME="Your name" -j$(nproc)
make epsilon.A_flash
```

Without the bootloader (legacy, deprecated):

```bash
make MODEL=n0110 GAMMA_USERNAME="Your name" -j$(nproc)
make MODEL=n0110 epsilon_flash
```

`GAMMA_USERNAME` is the preferred spelling. `OMEGA_USERNAME` is still accepted
for compatibility with old build scripts.

### Other targets

```bash
make PLATFORM=simulator TARGET=android -j$(nproc)   # epsilon.apk
make PLATFORM=simulator TARGET=3ds -j$(nproc)       # epsilon.3dsx
make PLATFORM=simulator TARGET=fxcg -j$(nproc)      # epsilon.g3a
make PLATFORM=simulator TARGET=macos -j$(nproc)
make PLATFORM=simulator TARGET=windows -j$(nproc)
```

Run `make help` for the full list.

## Installation

There is no official Gamma installer or hosted build yet. Build the firmware
yourself, verify the artifact, and flash it with the appropriate tool. Never
flash a binary from an untrusted source: a firmware has full access to the
calculator.

The bootloader keeps two slots. A bootloader build can be flashed into slot A or
B and keeps the other slot intact. The legacy n0110 build has no slot protection.

## Contributing

Contributions are welcome. Read [CONTRIBUTING.md](./CONTRIBUTING.md) before
opening a pull request. For a bug, use the issue templates. For a security
issue, follow [SECURITY.md](./.github/SECURITY.md).

The main development branch is `gamma-dev`; `gamma` is the stable branch. Do not
open pull requests directly against a release tag.

## License

Gamma is distributed under the
[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode)
license, like Epsilon 15.5.0, Omega and Upsilon.

- **Epsilon 15.5.0**: CC BY-NC-SA 4.0. This is the base of Gamma.
- **Epsilon 16.0.0 and later**: no `LICENSE` file; the upstream README states
  “All rights reserved”. Gamma does not include code from these versions.
- **Omega** and **Upsilon**: CC BY-NC-SA 4.0.

See [NOTICE.md](./NOTICE.md) for the full notice, trademarks and third-party
components.

## Links

- [Issue tracker](https://github.com/noctyris/Gamma/issues)
- [Discussions](https://github.com/noctyris/Gamma/discussions)
- [Build documentation](./docs/)
- [Contributing guide](./CONTRIBUTING.md)

NumWorks is a registered trademark of NumWorks SAS. Nintendo and Nintendo 3DS
are registered trademarks of Nintendo of America Inc. Casio is a registered
trademark of Casio Computer Co., Ltd. None of these companies is associated
with Gamma.

# Test matrix

Gamma tests are layered. A change is not considered verified just because it
compiles: the relevant layer below must pass, and platform gaps must be stated
in the pull request.

## Gates

| Gate | Required checks |
| --- | --- |
| Pull request | Linux simulator build, Linux test binary, shell/Python/workflow lint |
| Merge to `gamma-dev` | Pull-request gate plus n0110 device build and n0100 legacy build |
| Release | Merge gate plus every available host target, web build, artifact checksums and the manual hardware checklist |

## Targets

| Target | Build command | Current automation | Release requirement |
| --- | --- | --- | --- |
| Linux simulator | `make PLATFORM=simulator TARGET=linux` | CI on every push and PR | Yes |
| Linux tests | `make PLATFORM=simulator TARGET=linux test` | CI on every push and PR | Yes |
| n0110 with bootloader | `make MODEL=bootloader` | Local build; not in CI | Yes, build and flash test |
| n0110 legacy | `make MODEL=n0110 epsilon.dfu` | Local build; not in CI | Build only; deprecated flash path |
| n0100 legacy | `make MODEL=n0100 EPSILON_I18N=en epsilon.dfu` | Local build; not in CI | Yes, build; hardware optional |
| Web simulator | `make PLATFORM=simulator TARGET=web htmlpack.zip` | Not available without emsdk | Yes when emsdk is available |
| Android | `make PLATFORM=simulator TARGET=android` | SDK not present locally | When SDK is available |
| iOS | `make PLATFORM=simulator TARGET=ios` | macOS/Xcode not present | When SDK is available |
| macOS | `make PLATFORM=simulator TARGET=macos` | macOS not present | When SDK is available |
| Windows | `make PLATFORM=simulator TARGET=windows` | Cross-build not present | When toolchain is available |
| 3DS | `make PLATFORM=simulator TARGET=3ds` | devkitARM not present | When SDK is available |
| fxcg | `make PLATFORM=simulator TARGET=fxcg` | Toolchain not present | When SDK is available |
| nspire | `make PLATFORM=simulator TARGET=nspire` | Toolchain not present | When SDK is available |
| Real n0110 | Flash a locally built DFU | No device attached | Manual checklist |

## Layers

### Unit tests

`quiz` cases live next to the code and are compiled into
`output/release/simulator/linux/test.bin`. They cover parsing, layout,
simplification, statistics, regression, applications and storage. The runner
checks that the Poincaré tree pool is balanced after every case.

```bash
make PLATFORM=simulator TARGET=linux test -j"$(nproc)"
./output/release/simulator/linux/test.bin --headless
```

The last line must be `ALL TESTS FINISHED`.

### Golden tests

Deterministic reference vectors live in `poincare/test/golden.cpp`. Each vector
fixes the parsed expression, its serialization and its layout serialization.
They are compiled into the normal test binary, so CI runs them with every test
run.

Rules:

- Golden vectors are explicit input/output pairs, not generated from the code
  under test.
- A golden change must be intentional and reviewed with the behavioural change.
- Do not put locale-dependent text, timestamps, random values or absolute paths
  in a golden.
- Add a new vector when implementing a clean-room 23.x behaviour; keep the
  vector small enough to diagnose a failure.
- When a golden fails, fix the behaviour or update the reference with a written
  reason. Never weaken the assertion to hide a regression.

### Build matrix

Builds catch toolchain, flash-size and platform integration failures that unit
tests cannot. The n0100 link is especially sensitive to flash growth; it must be
built after application or translation changes.

### Manual hardware checklist

Run on a real n0110 when a release changes device code:

- boot into the bootloader, select slot A or B, and verify the About version;
- launch every built-in app, including the external-app menu;
- check display, keyboard, battery, LED, backlight and USB enumeration;
- verify save/restore of variables, functions and Python scripts;
- run the Grapher with a polar curve and a parametric curve;
- test USB DFU and the recovery path with a known-good image;
- verify exam mode if the release changes exam-mode code.

## Clean-room 23.x acceptance

The feature IDs refer to [CLEAN-ROOM-23.md](./CLEAN-ROOM-23.md).

| ID | Unit | Golden | Simulator | Hardware |
| --- | --- | --- | --- | --- |
| C1 derivative plots | Graph/function tests | Plot vectors once a deterministic renderer exists | Manual visual check | n0110 |
| C2 derivative notation | Parser + expression tests | Parse/serialize `f'`, `f''`, `f(n)` | Type in Calculation and Grapher | n0110 |
| C3 polar/parametric derivatives | Graph tests | Parameter/value vectors | Manual Find menu | n0110 |
| C4 polar/parametric storage | Graph/storage tests | Definition serialization | Save/reload and variable menu | n0110 |
| C5 polar grid | Range/options tests | Grid selection state | Visual check | n0110 |
| C6 search interval | Solver tests | Root vectors at interval bounds | Enter an explicit interval | n0110 |
| C7 inference from data | Statistics + inference tests | Mean/stddev and test vectors | Dataset import and calculation | n0110 |
| C8 turtle distance | Python port test | Numeric vector | Run a script | n0110 |
| C9 Python `@` | Python port/toolbox test | Toolbox insertion | Run a matrix example | n0110 |

## Known gaps

- No screenshot or framebuffer golden harness. Grapher and UI checks remain
  manual until a deterministic display harness exists.
- Web, Android, iOS, macOS, Windows, 3DS, fxcg and nspire are not built by the
  current Linux CI.
- No n0100 or n0110 hardware is attached to the development environment.
- `n0100` has no Atomic app and no external apps; its reduced set is intentional.

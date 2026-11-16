# Upstream 16.0.0 test merge

This document records a **no-commit test merge** of NumWorks Epsilon 16.0.0 into
Gamma. It is an analysis artifact, not an integration plan.

## Test setup

- Gamma base: `96501944b` (`Rebrand simulator targets and document Gamma`)
- Upstream commit: `9465a2e62912915035ae08047048b10dc79b055e` (tag `16.0.0`)
- Local branch: `upstream-sync/epsilon-16.0.0`
- Command: `git merge --no-commit --no-ff 9465a2e...`
- Worktree: temporary, under `output/upstream-sync`
- Result: **merge aborted cleanly; no merge commit was created**

The base branch is `gamma-dev`. The existing `upstream-sync/gamma-beta` branch was
not modified.

## Result

| Metric | Value |
| --- | ---: |
| Files changed by the merge | 1,915 |
| Unmerged paths | 1,455 |
| Merge commit created | 0 |

The conflict count alone makes a mechanical merge unrealistic. More importantly,
Epsilon 16.0.0 is published under “All rights reserved”; resolving these
conflicts would import unlicensed code. The merge must not be committed or
published.

## Conflicts by module

| Module | Unmerged paths | Risk | Main areas |
| --- | ---: | --- | --- |
| `apps` | 654 | Very high | App container, calculation, code, graph, probability, regression, statistics, settings, sequence, solver, home. |
| `poincare` | 212 | Very high | Parser, expression nodes, simplification, layout and tests; most headers and sources conflict. |
| `ion` | 198 | Very high | Device drivers, board files, USB, display, keyboard and n0100/n0110 layout. |
| `python` | 173 | Very high | MicroPython port, generated headers, modules and tests. |
| `escher` | 161 | Very high | GUI toolkit headers and sources. |
| `build` | 27 | High | Build system, device tools and platform files. |
| `kandinsky` | 15 | Medium | Fonts, context and drawing sources. |
| `liba` | 5 | Medium | Freestanding C library bridge and headers. |
| `docs` | 3 | Low | Architecture and build documentation. |
| `.github` | 3 | Low | Issue templates and CI workflow. |
| `libaxx` | 1 | Low | C++ runtime bridge. |
| Root files | 3 | Low | `README.md`, `Makefile`, `.gitignore`. |

### Main hotspots

| Path | Unmerged paths |
| --- | ---: |
| `ion/src` | 182 |
| `python/src` | 136 |
| `apps/shared` | 136 |
| `poincare/src` | 114 |
| `escher/include` | 80 |
| `escher/src` | 77 |
| `poincare/include` | 74 |
| `apps/code` | 59 |
| `apps/graph` | 58 |
| `apps/calculation` | 53 |
| `apps/probability` | 48 |
| `apps/regression` | 46 |
| `apps/statistics` | 40 |
| `apps/settings` | 33 |
| `apps/sequence` | 28 |
| `apps/solver` | 26 |
| `python/port` | 25 |
| `apps/hardware_test` | 24 |
| `poincare/test` | 23 |
| `apps/home` | 16 |

## Module assessment

### `apps`

The conflict is structural, not limited to a few files. The app container,
shared UI, calculation/graph/code features and nearly every app diverge at the
same time. Resolving this module would require reimplementing the upstream
behaviour against Gamma's application model; it is not a merge-resolution task.

### `poincare` and `escher`

Both libraries conflict across their public headers and implementations,
including parser, simplification, layout and GUI contracts. A partial merge
would create an inconsistent ABI and would make the simulator tests meaningless
until the whole module is reconciled.

### `ion`

Driver, board and simulator changes are interleaved. This is the module with the
highest hardware risk: a wrong resolution can compile but fail on a real n0110
or n0100. It must remain untouched without hardware testing.

### `python`

The MicroPython port and generated headers conflict heavily. Upstream generated
files cannot be copied into Gamma without regenerating them against Gamma's
configuration and reviewing the MicroPython version.

### `build`, `kandinsky`, `liba` and root files

These have fewer textual conflicts, but they are dependencies of the modules
above. Merging them alone would not produce a buildable tree.

## Decision

**Do not merge Epsilon 16.0.0.** The test merge is retained only as a conflict
inventory.

For any future upstream behaviour:

1. Identify the behaviour, not the patch.
2. Check the upstream license for that exact version.
3. Reimplement it cleanly in Gamma, without copying code, comments, assets or
   tests from an unlicensed version.
4. Add a regression test in the Gamma test suite.
5. Keep the simulator and n0110 builds green before considering a device test.

See [GAMMA.md](./GAMMA.md) for the build policy and compatibility matrix.

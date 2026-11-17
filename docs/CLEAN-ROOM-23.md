# Clean-room roadmap: Epsilon 23.2.3 behaviour

Gamma does not import Epsilon 16.0.0 or later. This document lists the
user-visible Epsilon 23 behaviours that Gamma may reimplement independently,
with a priority based on visibility and usage.

Reference: the public [NumWorks Version 23 release page](https://www.numworks.com/calculator/update/version-23/),
which describes the 23.x feature set. Version 23.2.3 is a patch release of that
series. This list is based on public documentation only; no upstream source,
tests, comments or assets were read or copied.

## Status in Gamma

| ID | Behaviour | Gamma status | Main code | Effort |
| --- | --- | --- | --- | --- |
| C1 | Plot first and second derivatives; colour, hide and value-only modes | **Partial**: values, banner and Cartesian table columns exist; derivative curves are not drawn | `apps/graph/graph/graph_view.cpp`, `apps/graph/values/values_controller.cpp`, `apps/shared/continuous_function.cpp` | M |
| C2 | Accept `f'`, `f''` and `f(n)`; Alpha+Toolbox inserts an apostrophe | **Absent**: `f'(x)` is only a display name; the parser has no derivative notation | `apps/shared/continuous_function.cpp`, `poincare/src/parsing/` | M |
| C3 | Derivative values and Find actions for polar and parametric curves | **Absent**: derivative UI is restricted to Cartesian functions | `apps/graph/graph/curve_parameter_controller.cpp`, `apps/graph/values/values_controller.cpp` | M |
| C4 | Polar/parametric definitions stored as `r1(θ)` and `f(t)=(x(t),y(t))`, templates and variable entries | **Partial**: plots, default names and type templates exist; verify storage and variable-menu parity | `apps/graph/list/`, `apps/shared/continuous_function.cpp` | S/M |
| C5 | Polar grid in the Grapher axis menu | **Absent**: the grid is Cartesian only | `apps/shared/curve_view.cpp`, Grapher range/options | S/M |
| C6 | Automatic search interval for equation solving | **Present**: editable search interval with a default `[-10, 10]`; verify the exact automatic behaviour of 23.x | `apps/solver/interval_controller.cpp`, `apps/solver/equation_store.cpp` | S |
| C7 | Inference from Statistics/Regression datasets, including sample mean and standard deviation | **Absent**: no Inference app or dataset import | new `apps/inference/`, `poincare/` statistics helpers | L |
| C8 | `turtle.distance()` | **Present**: `distance(x, y)`, catalog entry and simulator test | `python/port/mod/turtle/`, `apps/code/python_toolbox.cpp` | S |
| C9 | `@` operator in the Python toolbox | **Present**: exposed in the Python catalog and supported by ulab arrays | `apps/code/python_toolbox.cpp`, `python/port/mod/ulab/` | S |

## Priorities

Priority is by user visibility, not by implementation convenience.

### P0 - next minor release

1. **C2, derivative notation.** It changes what students can type in every
   non-Python application and unlocks the rest of the derivative work.
2. **C1, derivative plots.** High visibility in the Grapher; the value and table
   infrastructure already exists.
3. **C6, search interval parity.** Mostly validation and edge cases; do not
   rewrite the existing solver without a failing test.

### P1 - following release

4. **C7, inference from data.** Large, but the most visible missing application.
5. **C3, polar/parametric derivative values and Find.**
6. **C4 and C5, polar/parametric storage parity and polar grid.**

### P2 - opportunistic

7. **C8, `turtle.distance()`.** **Done** in the current development tree.
8. **C9, `@` in the Python toolbox.** **Done** in the current development tree.

## Clean-room protocol

For each item:

1. Write a short behaviour spec from the public release notes or manual:
   input, visible result, edge cases and error cases. Do not describe the
   implementation of the reference.
2. Add the failing test first. Use fixed golden vectors for deterministic
   parser, layout or numeric output.
3. Implement against Gamma's existing abstractions, not against an upstream
   patch. Do not copy code, comments, test vectors, icons, translations or
   assets from Epsilon 16+.
4. Keep the change inside one layer where possible: Poincaré for notation and
   math, Grapher for plots, a dedicated app for inference, Python port for the
   Python API.
5. Verify the Linux simulator and full test suite. Update the target build
   matrix for device or platform-specific changes.
6. Record the public reference and the limits of what was verified in the
   commit message.

## Acceptance criteria

- C2: parsing `f'`, `f''` and `f(n)` produces the same derivative expression as
  the equivalent `diff(...)` call; invalid orders are rejected without a crash;
  Alpha+Toolbox inserts `'` outside Python.
- C1: first and second derivative curves can be enabled, hidden and coloured;
  values remain correct when the derivative plot is hidden; table columns and
  the banner agree.
- C3: polar and parametric derivative values use the curve parameter, not `x`;
  `dy/dx` and tangent actions return the expected values at a selected point.
- C4/C5: definitions survive a save/reload and appear in the variable menu;
  the polar grid can be selected without changing the curve domain.
- C6: an equation with roots outside the default interval can use an explicit
  interval; the default interval is documented and tested.
- C7: a Statistics or Regression dataset can be selected or entered directly;
  sample mean and sample standard deviation match the statistics app; slope
  tests can select a regression dataset.
- C8: `turtle.distance(x, y)` returns the Euclidean distance from the current
  turtle position and does not draw.
- C9: the Python toolbox inserts `@` and a matrix multiplication example runs
  in the simulator.

## Non-goals

- Importing Epsilon 16+ code or generated files.
- Reimplementing every internal refactor of 23.x.
- Matching undocumented pixel positions, private storage layouts or exam-mode
  certification.
- Rewriting the Grapher or solver wholesale before a failing behavioural test
  justifies it.

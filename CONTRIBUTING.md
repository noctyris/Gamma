# Contributing to Gamma

Thanks for your interest in Gamma. This guide describes the workflow used by the
project.

## Branches

- `gamma` is the stable branch.
- `gamma-dev` is the integration branch. Open pull requests against it.
- Create a feature branch from `gamma-dev`, for example
  `fix/graph-derivative` or `feature/finance-tvm`.
- Do not open a pull request directly against a release tag.

## Before you start

For a small fix, open a pull request directly. For a large change, open an issue
first so the approach can be discussed.

Gamma is based on Epsilon 15.5.0. Do not copy code from Epsilon 16.0.0 or later:
those versions are published under “All rights reserved”. If you need behaviour
from a newer version, reimplement it cleanly and describe the reference.

## Build and test

Build the simulator:

```bash
make PLATFORM=simulator TARGET=linux -j$(nproc)
```

Build and run the tests:

```bash
make PLATFORM=simulator TARGET=linux test -j$(nproc)
./output/release/simulator/linux/test.bin --headless
```

Build the device firmware when relevant:

```bash
make MODEL=n0110 epsilon.dfu -j$(nproc)
```

Build the web simulator:

```bash
make PLATFORM=simulator TARGET=web htmlpack.zip -j$(nproc)
```

Do not commit files from `output/` or other generated directories.

## Versioning

The Gamma version is derived from git history. A regular commit increments the
patch number; an integration merge increments the minor number. Do not edit a
version file by hand. For a release, pass `GAMMA_VERSION_MAJOR` or the full
`GAMMA_VERSION` on the make command line.

## Coding style

- Follow the style of the surrounding code.
- Keep the existing C++ standard of the file you edit unless the change
  explicitly updates it.
- Prefer focused commits; avoid reformatting unrelated lines.
- Add or update tests when the change affects behaviour.
- Update the `.i18n` files when you add a user-visible string.

## Commit messages

Use a short component prefix, matching the existing history:

```text
[apps/graph] Fix derivative colour on implicit curves
[ion] Add Gamma version to the platform header
[build] Require an explicit target for merge checks
```

Write the subject in the imperative mood and explain the why in the body when it
is not obvious.

## Pull requests

A pull request should:

- be based on `gamma-dev`;
- contain a focused change;
- describe how it was tested;
- include the checklist from the pull request template;
- update documentation when behaviour changes.

## License

By contributing, you agree that your contribution is distributed under the same
CC BY-NC-SA 4.0 license as the project, and you confirm that you have the right
to submit it. See [NOTICE.md](./NOTICE.md).

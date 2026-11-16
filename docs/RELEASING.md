# Releasing Gamma

Gamma versions are derived from git history. A release is a commit whose
`make info` output is the version being published, plus an annotated tag.

## Version rules

- Every regular commit increments the patch number.
- An integration merge increments the minor number and resets the patch.
- `GAMMA_VERSION_MAJOR` or a full `GAMMA_VERSION` override is for an explicit
  release decision.

The existing tags are `gamma-v1.1.0` and `gamma-beta-final`. The next tag must
match the version printed at the tagged commit; do not tag a commit and then
add release-only commits afterwards.

## Preflight

```bash
git status --short
make info
make PLATFORM=simulator TARGET=linux -j"$(nproc)"
make PLATFORM=simulator TARGET=linux test -j"$(nproc)"
./output/release/simulator/linux/test.bin --headless
```

The working tree must be clean and the test binary must end with
`ALL TESTS FINISHED`.

## Build artifacts

Build the targets available in the release environment:

```bash
make MODEL=bootloader GAMMA_USERNAME="Gamma" -j"$(nproc)"
make MODEL=n0110 GAMMA_USERNAME="Gamma" epsilon.dfu -j"$(nproc)"
make MODEL=n0100 EPSILON_I18N=en GAMMA_USERNAME="Gamma" epsilon.dfu -j"$(nproc)"
make PLATFORM=simulator TARGET=linux -j"$(nproc)"
# If emsdk is installed:
make PLATFORM=simulator TARGET=web htmlpack.zip -j"$(nproc)"
```

Record the artifact paths and SHA-256 checksums. Do not publish a firmware that
was not built from the tagged commit. Never publish a binary from an untrusted
source.

## Tag and release

Create the annotated tag locally only after the release commit is final:

```bash
VERSION="$(make info | awk -F' = ' '/^GAMMA_VERSION = / {print $2}')"
git tag -a "gamma-v${VERSION}" -m "Gamma ${VERSION}"
git show --stat "gamma-v${VERSION}"
```

The tag annotation is the release note of record. It should summarise:

- the user-visible changes since the previous tag;
- the tested targets and any target that could not be tested;
- the known limitations, especially n0100 and any untested hosted port;
- a pointer to `docs/GAMMA.md`, `docs/TEST-MATRIX.md` and
  `docs/CLEAN-ROOM-23.md`.

Pushing the commit and the tag, creating the GitHub Release, uploading
artifacts and changing repository settings are deliberate remote actions and
are not part of a local preparation.

## Post-release

- Keep the previous DFU image available for recovery.
- Do not move or delete a published tag.
- If a release is broken, publish a new patch release rather than rewriting
  history.
- Add the next clean-room items to the milestone before starting new work.

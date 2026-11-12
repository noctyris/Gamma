## Summary

Describe what this pull request changes and why.

## Type of change

- [ ] Bug fix
- [ ] New feature
- [ ] Build, CI or tooling
- [ ] Documentation
- [ ] Refactor or cleanup

## How has this been tested?

Describe the commands you ran and the result. At minimum, build the relevant
target and run the tests that cover the change.

```bash
make PLATFORM=simulator TARGET=linux -j$(nproc)
make PLATFORM=simulator TARGET=linux test -j$(nproc)
./output/release/simulator/linux/test.bin --headless
```

## Checklist

- [ ] My changes are focused and do not include unrelated formatting or generated files.
- [ ] I built the simulator and ran the tests.
- [ ] I built the device firmware when the change affects it (`make MODEL=n0110 epsilon.dfu`).
- [ ] I updated the documentation and translations when needed.
- [ ] I did not import code from Epsilon 16.0.0 or later.
- [ ] I have the right to submit this code under the project license.

## Related issues

Closes #

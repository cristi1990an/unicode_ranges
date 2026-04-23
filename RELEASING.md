# Releasing

This repository currently ships source-first releases.

For `1.x`, the supported release payload is:

- a Git tag
- GitHub source archives for that tag
- the first-party CMake install/export package produced from source

Prebuilt binaries are not part of the current release contract.

## Release Checklist

Before tagging a release:

1. Confirm the target branch is green in CI.
2. Confirm the project version in `CMakeLists.txt`.
3. Confirm `CHANGELOG.md` has a dated release entry.
4. Confirm `STABILITY.md`, `README.md`, and docs reflect the same support story.
5. Confirm the release-validation CI job passes:
   - configure
   - build
   - test
   - install
   - consumer `find_package(...)` smoke test
6. Confirm the compile-time probe job has recent accepted results.
7. Confirm the release notes draft exists under `release-notes/`.

## Tagging

Example for `v1.0.0`:

```powershell
git switch main
git pull --ff-only
git tag -a v1.0.0 -m "unicode_ranges v1.0.0"
git push origin v1.0.0
```

## GitHub Release Payload

When creating the GitHub release:

- title it `vX.Y.Z`
- use the matching draft from `release-notes/X.Y.Z.md`
- attach no binary artifacts unless the support policy changes
- keep the release marked as the first stable source release for `1.x` when applicable

# HoloNight Thumbnails

`holonight-thumbnails` is a synchronous Qt library for the user thumbnail cache defined by the [freedesktop thumbnail managing standard](https://specifications.freedesktop.org/thumbnail/latest-single/). It shares disk entries between HoloNight Files and Viewer. Source access, image decoding, jobs, and memory caches stay with callers.

```cmake
find_package(HolonightThumbnails 0.1 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE HolonightThumbnails::Thumbnails)
```

Include `<holonight_thumbnails/cache.h>`. Construct a `HolonightThumbnails::Request` with an absolute local file URI, source modification time and size, and the minimum useful output pixel size. Call `lookup` after verifying and inspecting the source. On a miss, decode in the caller and pass the bounded result to `store`. Check cancellation between these operations. Set `kind = Kind::Svg` only after validating the source against the static, self-contained SVG policy. Set `tier` to 128, 256, 512, or 1024 to force a tier; zero selects the smallest tier that contains `required`. A request larger than 1024 pixels remains memory only.

Raster entries with required standard metadata are reusable even when another application wrote them. Existing Files entries remain reusable when their policy tags are valid. SVG entries require the static SVG policy marker. The package never writes failure thumbnails or prunes old entries.

Build and test with installed Qt >=6.11 Core/Gui and GTest >=1.14 CONFIG (testing only):

```sh
task deps PRESET=test
cmake --preset test
cmake --build --preset test --parallel 2
ctest --preset test
```

Normal configure/build/test commands validate prerequisites without acquiring packages or preparing
providers. This library has no internal providers. `task deps` explicitly records installed prerequisite
evidence. Set `HOLONIGHT_EXTERNAL_PREFIX` for an external installation, including paths containing spaces.

## Standalone developer tooling

See [tooling/README.md](tooling/README.md) for presets, local dependency overrides, editor refresh,
`task tooling:doctor`, and the independent Serena project.


## Local CI rehearsal

Run `task ci:prepare` explicitly to acquire the qualified pinned linux/amd64 Arch and REUSE images
and record installed prerequisite/toolchain identities. Docker or Podman is required. Preparation needs
registry access; subsequent `task ci` acceptance runs offline with `--pull=never --network=none`.
Missing, stale or modified preparation fails before acceptance starts; incompatible evidence is preserved.

Four shared local/hosted lanes cover Debug provider tests and both CTest registrations, Release installation
and an external-prefix consumer with spaces in its paths, canonical format/strict tidy, and REUSE.
Caller UID/GID, rootless Podman keep-id, private runtime/cache directories, read-only source/preparation
mounts and fresh writable build trees isolate acceptance. Parallelism defaults to two.

Tracked edits and non-ignored new files enter snapshots; untracked inputs are reported to add before
pushing. Full logs, artifacts, revision/dirty state and environment identities remain under ignored
`build/ci/` and `build/ci-prepared/`. Development builds remain independent. Run launcher regressions
with `python3 scripts/ci/test_launcher.py`; real discovery regressions run in the qualified build image.
See [the dependency SDD](docs/sdd/external-dependency-management/SPEC.md) for evidence and limitations.
Publication and submodule pinning require separate handoffs.

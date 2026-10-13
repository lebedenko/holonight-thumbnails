# External dependency management — holonight-thumbnails

Work package: EDM-010
Audit baseline: `2284b1822b0f8f856677e14d21d91e8fa10bd98c`
Implementation baseline: `59fb2e92e8794e06db2ff8a1a6ec5cc1ebc66c03`
Ready checkpoint: `91e3429`
Umbrella audit: `38146d8f474317f3f8d72699f60e6d16a71dd165`
Status: Verified locally; publication and gitlink update pending. Publication and pinning are separate handoffs.

## Requirements

- Configuration and normal build commands shall never acquire dependencies or prepare source providers.
- `task deps` shall explicitly check installed prerequisites and record evidence; Thumbnails has no internal providers.
- Internal providers shall resolve only from approved prefixes; external packages shall resolve from an explicit
  external prefix or normal system locations.
- Enabled dependency-dependent features shall require their dependencies; minimal profiles shall disable them explicitly.
- Where production components use Qt, discovery shall require >=6.11; private Qt components shall match
  the selected public version exactly.
- Where tests use GTest, consumers shall require installed GTest >=1.14 CONFIG and any GMock targets they use.
- Existing public APIs, component switches, install-tree consumers and acceptance coverage shall be preserved.

## Design

Adopt canonical `tooling/modules/holonight-thumbnails.json` and `.cmake`, then synchronize bundle 2.0.0 into this repository.
The copied metadata is owned by this standalone checkout. CMake performs authoritative discovery; shared probes
provide prerequisite diagnostics and record resolved versions/paths. Incompatible caches and local provider install
trees are archived before reuse. Package acquisition occurs only in explicit environment preparation. CI switches
build/runtime/release lanes together to qualified immutable images and runs acceptance offline after preparation.

Preserve existing dependency interfaces and component switches; adopt installed GTest CONFIG discovery where tests use it.

## Implementation and verification tasks

- [x] Confirm the exact baseline and canonical handoff before implementation.
- [x] Adopt declarations, prefix helpers, explicit preparation and compatibility signatures.
- [x] Apply the repository-specific discovery changes above.
- [x] Update Task, CI preparation/entrypoints, Dockerfiles, release lanes and contributor instructions.
- [x] Run declaration/CMake/profile consistency checks and offline missing-package regressions.
- [x] Run narrow regressions, clean acceptance, installed-package consumers and required quality checks.
- [x] Inspect complete logs; record commands, versions, features, warnings and manual checks.
- [x] Make a repository-local commit only after local verification; publish only when authorized.

## Adopted implementation

On 2026-10-13 the clean checkout and canonical `origin/main` were confirmed at the implementation
baseline using `git ls-remote origin refs/heads/main`. The historical audit baseline remains above.
The umbrella initiative remains Accepted; downstream adoption and ecosystem integration are outside EDM-010.

- `tooling/module.json` and `dependencies.cmake` copy the canonical Thumbnails declarations. Bundle 2.0.0
  updates workflow/README/VERSION and adds external.py, external-packages.json and providers.cmake.
- CMake includes the dependency helper and requires installed GTest >=1.14 CONFIG only when testing is
  enabled. Qt >=6.11 Core/Gui discovery, all presets, package 0.1.0, exported headers and
  `HolonightThumbnails::Thumbnails` remain unchanged. Unused QML_IMPORT_PATH preset variables are removed.
- Task adds explicit ci:prepare. deps checks installed prerequisites; ordinary configure/build/test
  never acquire packages or prepare dependencies. Explicit external prefixes and paths with spaces work;
  incompatible configuration caches are archived before reuse.
- scripts/ci/run.py, lane.sh and images.json adapt the completed Images pattern: explicit compatible
  preparation, archived incompatible evidence, Debug build-test, Release staging/consumer, static-checks,
  and licensing. Acceptance uses --pull=never --network=none, read-only snapshots and preparation,
  caller UID/GID, Podman keep-id, private HOME/XDG/runtime directories and parallelism two by default.
- The existing ci.yml explicitly prepares and runs all lanes, uploading full evidence. Contributor README
  and historical local-CI TASKS link this contract. tests/package-test.cmake exports compile commands for
  the original installed consumer so strict tidy covers all three owned translation units.

## Separate quality cleanup

Canonical format and strict tidy exposed pre-existing owned diagnostics. A separate C++ cleanup commit
adds braces, checked QByteArray byte access and explicit unsigned casts in PNG inspection, a simplified
revision comparison, a static image helper, a const fixture writer and reserved writer-vector capacity.
Fixture data is public for generated GTest subclasses; lifecycle methods remain protected. Public enums
retain their representation via targeted performance-enum-size annotations. A targeted complexity
annotation keeps ordered cancellation checkpoints and tier fallback together in lookup. No canonical
policy is disabled globally. Public header declarations match the baseline after stripping comments and
whitespace; all 54 existing test/assertion macro occurrences remain in order, including all 11 cases.

## Environment and focused verification

Qualified EDM-024 full Arch image:
`ghcr.io/lebedenko/holonight-deps@sha256:37ca051c39d1688d6413c47d94b8c0f221152dac6981ec67d155f2c43ea17b40`.
Pinned REUSE image:
`docker.io/fsfe/reuse@sha256:85462a75c0f8efda09ddd190b92816b70e7662577c8427429e11e1b9f25a992e`.
GCC 16.2.1, CMake 4.4.3, Clang/clang-tidy 22.1.8, Python 3.14.7, Ninja 1.13.2,
Qt Core/Gui 6.11.2 and GTest 1.18.0 are installed. Docker runs are real; rootless Podman mapping
is regression-tested with a fake runtime, with no claim of real Podman acceptance.

`python3 scripts/ci/run.py --prepare` passed at build/ci/20261013T111731Z-czf2hazj.
Initial focused build-test passed both CTest entries and all 11 cache cases at
build/ci/20261013T113457Z-cpo_pajz, while static-checks exposed inherited format failures.
After formatting, strict tidy failures were retained at build/ci/20261013T122110Z-yo97wbas and
build/ci/20261013T122208Z-cwl58hle. Corrected static checks passed at
build/ci/20261013T122254Z-6dim1wh0. Full focused logs were reviewed.
Canonical tooling's 32 fixtures, declaration equality, bundle drift and installer-profile consistency pass.

Launcher regressions cover missing/stale/tampered preparation, incompatible evidence preservation,
offline mounts, read-only inputs/preparation, rootless user mapping, snapshot modes/symlinks/spaces,
and failure propagation through all lanes. Real discovery regressions cover unavailable/old Qt and
GTest, testing disabled without GTest, explicit installed GTest prefixes, spaces, compatible cache reuse,
changed-prefix archival and normal commands succeeding/failing without preparation or acquisition.

## Diagnostics and limits

Optional Vulkan-header discovery notices originate in Qt and do not affect CPU cache checks.
Strict tidy suppresses non-user Qt/standard-library diagnostics and only the targeted enum/complexity
annotations above. Failed fake-runtime lanes are intentional passing launcher regression scenarios.
No UI interaction is needed for this library. Real Podman, Debian qualification, Viewer/Files adoption,
manual ecosystem checks, publication and gitlink updates remain separate work.

## Clean acceptance — 2026-10-13

Final focused build-test passed at `build/ci/20261013T122354Z-dlv_ytxi`.
One clean `python3 scripts/ci/run.py` acceptance passed all four lanes at
`build/ci/20261013T122552Z-hkum50st`. Complete logs were reviewed:

- Debug provider: both CTest registrations pass; all 11 cache cases run without skips, retaining
  permissions, corruption, metadata/tier/hash checks, cancellation, concurrency and write-failure misses.
- All eight launcher and eleven real discovery/cache regressions pass in the offline qualified image.
- Release stages exactly eight expected files under /usr. The separate installed consumer builds/runs
  using `/work/external prefix` and `/work/consumer build`; no tests are installed.
- Canonical format and strict tidy pass across the three owned translation units: cache.cpp,
  cache_test.cpp and the original installed consumer. No unresolved owned diagnostics remain.
- REUSE 6.2.0 passes in the separately pinned image. Final documentation/evidence licensing is checked
  again before committing; this does not invalidate the application lanes.

Declaration JSON/CMake equality, bundle drift, installer-profile consistency, Python/shell syntax,
hosted workflow YAML and whitespace checks pass. Final code matches the clean acceptance snapshot;
subsequent changes only finalize documentation and evidence. Full preparation identity/package evidence
is in [environment.json](verification/environment.json); acceptance report, log hashes, staged inventory
and implementation hashes are in [acceptance.json](verification/acceptance.json). Full logs/artifacts
remain in ignored build/ci; initial failures are preserved for audit. No manual UI checks are required.
Local cleanup/adoption commits remain unpublished, and the umbrella gitlink remains at the baseline.

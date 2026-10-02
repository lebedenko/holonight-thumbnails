# Local CI rehearsal

Baseline: b545c76b8c852b3023a7f5bfc3d468a0667c221d. Umbrella package CI-005.

Add the missing push/PR/manual build-validation job and task ci around shared
repository-owned scripts. Use the immutable Qt6/compiler image for a fresh
Debug/Ninja build with compile commands and BUILD_TESTING=ON, followed by all
provider and installed-package CTest cases (output-on-failure, no-tests=error).
No external provider is required. Preserve cache behavior and development tasks.

Snapshot tracked edits and non-ignored new inputs, omit deleted/ignored files,
preserve modes and symlinks, and report untracked inputs to add before pushing.
Containers mount input read-only and use disposable writable source/build trees;
source and normal development builds must stay unchanged. Save full logs, source
revision/dirty state, image identity, tool versions and lane results in build/ci.
Docker is preferred; absent Docker falls back to Podman, keeping rootless UID/GID
mapping. All required failures or unavailable runtimes must return nonzero.
Publication, pushes and pin updates are excluded. Local acceptance passed on 2026-10-03.


Verification: clean task ci passed in build/ci/20261002T222457Z-19incb06/
with both provider and installed-package CTest registrations. Four launcher
regressions passed, including private user mapping, missing runtimes and failure
propagation. Hashes/modes/timestamps confirmed 41 source/development-build files
stayed unchanged. Complete logs and final diff were reviewed; compiler warnings
were absent and optional Vulkan headers are not needed by the cache checks.
Real Podman execution is unverified because Podman is not installed on this host.

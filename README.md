# HoloNight Thumbnails

For current build, test, container acceptance and provider workflows, see [DEVELOPMENT.md](DEVELOPMENT.md).

`holonight-thumbnails` is a synchronous Qt library for the user thumbnail cache defined by the [freedesktop thumbnail managing standard](https://specifications.freedesktop.org/thumbnail/latest-single/). It shares disk entries between HoloNight Files and Viewer. Source access, image decoding, jobs, and memory caches stay with callers.

```cmake
find_package(HolonightThumbnails 0.1 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE HolonightThumbnails::Thumbnails)
```

Include `<holonight_thumbnails/cache.h>`. Construct a `HolonightThumbnails::Request` with an absolute local file URI, source modification time and size, and the minimum useful output pixel size. Call `lookup` after verifying and inspecting the source. On a miss, decode in the caller and pass the bounded result to `store`. Check cancellation between these operations. Set `kind = Kind::Svg` only after validating the source against the static, self-contained SVG policy. Set `tier` to 128, 256, 512, or 1024 to force a tier; zero selects the smallest tier that contains `required`. A request larger than 1024 pixels remains memory only.

Raster entries with required standard metadata are reusable even when another application wrote them. Existing Files entries remain reusable when their policy tags are valid. SVG entries require the static SVG policy marker. The package never writes failure thumbnails or prunes old entries.

Build and test with installed Qt >=6.11 Core/Gui and GTest >=1.14 CONFIG (testing only):

See [DEVELOPMENT.md](DEVELOPMENT.md) for current native and container commands.

## Isolated cache reads

Build with `HOLONIGHT_THUMBNAILS_ISOLATED=ON` against installed isolated Images,
then request `find_package(HolonightThumbnails CONFIG REQUIRED COMPONENTS Isolated)`
and link `HolonightThumbnails::Isolated`. Include `holonight_thumbnails/isolated.h`.
`lookupIsolated(request, client, cancelled, stage)` borrows the caller's explicitly
configured `HolonightImages::IsolatedClient` and returns an optional `image`, a
separate `runtime` status and `diagnostic`. A successful runtime with no image is
a cache miss. Runtime failures and cancellation stop tier traversal; callers must
not turn them into source-decoding fallbacks. Cleanup failure takes precedence.

The isolated and legacy APIs share traversal, freshness, tier and text identity
validation. PNG reads retain the 32 MiB encoded limit and tier-specific pixel
bounds, including SVG-policy entries. Malformed/unsupported entries remain misses.
Only isolated decoding uses the helper; writes and the legacy API are unchanged.
The default target does not acquire an Images dependency. Native test coverage can
be enabled with `HOLONIGHT_THUMBNAILS_TEST_RUNTIME` pointing at a prepared bundle.

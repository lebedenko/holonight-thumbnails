# HoloNight Thumbnails

`holonight-thumbnails` is a synchronous Qt library for the user thumbnail cache defined by the [freedesktop thumbnail managing standard](https://specifications.freedesktop.org/thumbnail/latest-single/). It shares disk entries between HoloNight Files and Viewer. Source access, image decoding, jobs, and memory caches stay with callers.

```cmake
find_package(HolonightThumbnails 0.1 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE HolonightThumbnails::Thumbnails)
```

Include `<holonight_thumbnails/cache.h>`. Construct a `HolonightThumbnails::Request` with an absolute local file URI, source modification time and size, and the minimum useful output pixel size. Call `lookup` after verifying and inspecting the source. On a miss, decode in the caller and pass the bounded result to `store`. Check cancellation between these operations. Set `kind = Kind::Svg` only after validating the source against the static, self-contained SVG policy. Set `tier` to 128, 256, 512, or 1024 to force a tier; zero selects the smallest tier that contains `required`. A request larger than 1024 pixels remains memory only.

Raster entries with required standard metadata are reusable even when another application wrote them. Existing Files entries remain reusable when their policy tags are valid. SVG entries require the static SVG policy marker. The package never writes failure thumbnails or prunes old entries.

Build and test with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build/debug -j4
ctest --test-dir build/test --output-on-failure
```

## Standalone developer tooling

See [tooling/README.md](tooling/README.md) for presets, local dependency overrides, editor refresh,
`task tooling:doctor`, and the independent Serena project.

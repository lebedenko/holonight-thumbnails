# Shared thumbnail cache provider

## Status

Implemented locally. Work package THC-001.

## Requirements

- Provide an independently installed Qt6 CMake package with synchronous lookup and store.
- Use the freedesktop personal thumbnail cache path, canonical file URI MD5 key, and 128/256/512/1024 pixel tiers.
- Treat stale, corrupt, unavailable, or cancelled entries as misses. Bound PNG input size and pixel dimensions before decode.
- Accept standard raster metadata from external applications; check HoloNight and legacy Files policy metadata whenever present.
- Require a static, self-contained SVG policy marker. Consumers validate SVG source content before lookup.
- Use private directories and files, and atomic same-directory replacement.
- Keep source verification, decoding, scheduling, memory cache, and UI in consumers.

## Design

The public `Request` carries the URI, source mtime and size, minimum useful decoded dimensions, content kind, optional Files revision, and optional explicit tier. `lookup` searches the selected tier and larger tiers. `store` writes only the selected tier. Both return a miss/failure on cancellation or cache I/O errors. Callers retain their decoded source image on a failed store.

PNG entries carry the standard `Thumb::URI`, `Thumb::MTime`, and `Thumb::Size` fields. New entries also carry millisecond mtime and orientation policy extensions. Files policy tags are written for compatibility. Standard raster entries may omit `Thumb::Size`; when present it must match. The standard's whole-second mtime limit remains for external entries without stronger metadata.

Raster stores privately select PNG compression 11 for directional variation. RGB32, ARGB32,
ARGB32 premultiplied, RGBX8888, RGBA8888 and RGBA8888 premultiplied are compared as stored
32-bit pixels, without conversion or padding. For dimensions at least two, the detector
uses `nx=min(32,w-1)`, `ny=min(32,h-1)` and midpoint coordinates
`x=1+floor((2i+1)(w-1)/(2nx))` (equivalently for y). It counts pixels differing from
both their left and upper neighbors and selects compression 11 exactly when
`4*count >= nx*ny`. It scans the complete grid and checks cancellation before each
sampled row and again before encoding. SVG, unsupported formats, tiny images and
below-threshold images retain the existing default writer path.

Classification stays inside synchronous `store`; metadata, color, permissions,
cancellation, atomic replacement and publication order are unchanged. Existing entries
remain valid. The helper and its cancellation probe are private; there are no public
settings, package changes or cache migrations. Selection evidence and coordinated
rollout are recorded in the umbrella directional encoding rollout note.

The library avoids thumbnails for source URIs inside the personal thumbnail cache. Its callers must first verify that the original source is readable. Disk pruning and shared `.sh_thumbnails` repositories are outside this work package.

## Verification

`ctest --test-dir build --output-on-failure` exercises cache keys and tiers, standard raster reuse, source change, corrupt and undersized entries, SVG policy, cancellation before atomic commit, concurrent writers, write failure, cache source exclusion, and an installed package consumer.

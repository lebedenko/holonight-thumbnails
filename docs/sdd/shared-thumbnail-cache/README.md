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

The library avoids thumbnails for source URIs inside the personal thumbnail cache. Its callers must first verify that the original source is readable. Disk pruning and shared `.sh_thumbnails` repositories are outside this work package.

## Verification

`ctest --test-dir build --output-on-failure` exercises cache keys and tiers, standard raster reuse, source change, corrupt and undersized entries, SVG policy, cancellation before atomic commit, concurrent writers, write failure, cache source exclusion, and an installed package consumer.

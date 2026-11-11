# Transaction-backed cache lookup

The `lookupIsolated` overload taking an Images `SourceTransaction` reuses the
existing candidate traversal, filesystem restrictions, identity tags, adequacy
checks and fallback order. It delegates bounded PNG decoding to the transaction.
Images owns transport and decoding; Thumbnails owns cache filesystem policy.
The result is provisional and may only coordinate selection inside the callback.
A disk miss does not block captured-source decoding. Infrastructure exceptions
terminate the enclosing transaction instead of becoming cache misses.

The standalone isolated lookup overload and in-process cache API are unchanged.
Cache writes remain explicit and occur only after successful transaction cleanup.

See the [Images design](../../../../holonight-images/docs/sdd/source-preview-transaction/DESIGN.md)
and [umbrella contract](../../../../docs/initiatives/source-preview-transaction.md).

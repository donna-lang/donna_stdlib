# Changelog

All notable changes to `donna_stdlib` will be documented in this file.

## Unreleased

## [0.3.0] — 2026-05-19

### Added

- `io` module: `print`, `println`, `eprint`, `eprintln`, `flush`, `flush_stderr`, `read_line`, `read_all_stdin`.
- `path` module: `separator`, `join`, `join_all`, `extension`, `drop_extension`, `basename`, `dirname`, `is_absolute`, `is_relative`, `normalize`, `relative`.
- `files.mkdir_all` — create a directory and all missing parents.
- `files.remove_dir` — remove an empty directory.
- `files.remove_all` — remove a file or directory tree recursively.
- `files.rename` — rename or move a file or directory.
- `float.equal` — exact float equality comparison.
- `float.near` — approximate float comparison within a tolerance.
- `float.is_nan`, `float.is_infinite`, `float.is_finite` — float classification.
- `float.sign` — return -1, 0, or 1 for the sign of a float.
- `time.monotonic_ms`, `time.monotonic_us` — monotonic clock access (renamed from `now_ms`/`now_us`).
- `time.unix_seconds`, `time.unix_ms` — wall-clock time since Unix epoch.
- `time.sleep_ms` — sleep for a given number of milliseconds.

### Changed

- `time.now_ms`, `time.now_us` are now aliases for `time.monotonic_ms`, `time.monotonic_us`.

## [0.2.0] — 2026-05-17

### Changed
  * Optimized `string.index_of`, `string.index_of_from`, `string.contains`, and `string.replace` with C-backed scanning helpers while keeping the Donna API unchanged.

## [0.1.0] — 2026-05-07

Initial release.

### Added
  * Core modules: `bool`, `dict`, `files`, `float`, `int`, `list`, `option`, `result`, `shell`, `string`, and `time`
  * File helpers for reading, writing, appending, deleting, copying, path handling, and directory listing
  * Shell helpers for command execution, output capture, environment lookup, and stdout flushing
  * String helpers for slicing, trimming, splitting, joining, replacing, padding, conversion, and predicates
  * List helpers for mapping, filtering, folding, ranges, searching, zipping, flattening, and aggregation
  * Dictionary helpers for insertion, lookup, deletion, folding, mapping, filtering, merging, and conversion
  * Integer, float, boolean, option, and result utility functions
  * Time helpers: `time.now_ms` and `time.now_us`
  * Module and function documentation comments for docgen
  * Function documentation examples for generated docs
  * Self tests covering the public stdlib API

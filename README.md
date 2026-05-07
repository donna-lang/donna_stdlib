# donna_stdlib

<img src="https://img.shields.io/badge/Donna-stdlib-FF6347?style=for-the-badge" alt="Donna stdlib"/>

<a href="https://donna-lang.github.io/donna_stdlib/">
  <img src="https://img.shields.io/badge/Docs-Read-2F81F7?style=for-the-badge" alt="Docs - Read"/>
</a>

The Donna standard library.

## Overview

`donna_stdlib` provides the core modules used by Donna projects:

- `bool` for boolean helpers
- `dict` for small association-list dictionaries
- `files` for filesystem access
- `float` for floating-point helpers and math bindings
- `int` for integer helpers
- `list` for list processing
- `option` for optional values
- `result` for fallible values
- `shell` for command execution
- `string` for string processing
- `time` for monotonic clock measurements

## Installation

Add to your `donna.toml` as a dependency:

```toml
[dependencies]
donna = { git = "https://github.com/donna-lang/donna_stdlib", version = ">=0.1.0 and  <1.0.0" }
```

## Usage

```donna
import donna/string
import donna/list
import donna/time

pub fn main() -> Nil:
  let title = string.to_slug("Hello Donna")
  let total = list.sum([1, 2, 3])
  let started = time.now_ms()
  echo title
  echo string.from_int(total)
  echo string.from_int(time.now_ms() - started)
```

## API

For API Reference visit the generated docs [here](https://donna-lang.github.io/donna_stdlib/)


## Licence

MIT

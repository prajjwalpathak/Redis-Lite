## Overview

Redis-Lite is a lightweight, Redis-inspired in-memory key-value store implemented in modern C++.
It supports TTL expiration, thread safety, and configurable eviction policies (LRU & LFU).

## Features

- In-memory key-value storage
- Optional TTL per key
- LRU eviction policy
- LFU eviction policy
- Thread-safe API
- Clean PIMPL-based design
- Zero external dependencies

## Design Highlights

- Uses PIMPL idiom to keep public API stable
- Eviction policies implemented inside core store for performance
- TTL handled via <code>std\::chrono\::steady_clock</code>
- Expired keys lazily cleaned on access
- Mutex-protected critical sections

## Architecture

```text
KVStore (public API)/
└── Impl (private)/
    ├── unordered_map<string, Entry>
    ├── LRU list / LFU counters
    ├── eviction logic
    └── mutex
```

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Testing

Basic unit tests validate:
- CRUD operations
- TTL expiration
- LRU eviction
- LFU eviction
- Thread safety under concurrent access
- Verified memory safety with Valgrind, ensuring zero memory leaks and no use-after-free errors
- **Stress Testing**: Validated Redis-Lite using 150k+ randomized SET/GET/DEL operations with mixed TTLs under LRU and LFU eviction, asserting invariants and confirming zero memory leaks via Valgrind.
 
## Future Improvements

- Background eviction thread
- Persistence (RDB/AOF style)
- Networked server (RESP protocol)
- Sharding & replication

## Execute

- To run tests:
```bash
cd build
./redis_lite_tests
./redis_lite_stress_tests
```

- To execute example code:
```bash
cd build
./redis_lite_example
```
# Files created by Theia

This document serves as an overview of all the files created by the Theia runtime. Generally, these files are used to cache computations to increase the performance of the Theia runtime on subsequent runs. Some of these files will only be used if [configured](../configs/runtime-config.md).

### `%PROGRAMDATA%\Packer\catcache*.bin`

- **Type**: Catalog cache.
- **File name**: `catcache*.bin`, where `*` is usually a version number.  
- **File size**: 100 kB - 1 MB, depending on number of installed drivers on user's machine.
- **Purpose**: Stores a lookup table computed from the user's installed catalog files for signature checking purposes.
- **Associated configuration option**: Only used if `signature_check` is enabled. Always required if signature checking is enabled.
- **Notes**: The catalog cache will be automatically generated on first runs, or when the set of installed catalog files has changed. This generation will be done upon launch and may take between 5 and 20 seconds depending on hardware speed. Subsequent startups are instant.

  Launching the Theia `runtime.dll` as an executable while adding `PACKER_FUNCTIONALITY=ĂĀĀĀ` (special characters are intentional) as command-line argument will cause Theia to regenerate the catcache and then exit. If your application has an installer, it is recommended that you execute this step as part of installations and updates to avoid potential long first-startup times.

### `%PROGRAMDATA%\Packer\bhcache*.bin`

- **Type**: Authentihash cache
- **File name**: `bhcache*.bin`, where `*` is a build-specific identifier.
- **File size**: 0 B - 32 kB, depending on number of unique DLLs loaded by the protected application.
- **Purpose**: Stores a cache of cryptographic hashes for loaded DLLs to avoid recomputing these upon startup.
- **Associated configuration option**: Only used if `enable_hashcache` is set. Can still lead to performance improvements even if `signature_check` is disabled (as Theia always checks the cryptographic signature of itself).
- **Notes**: Primarily used to improve startup performance. If the hashcache is enabled, signature checking is usually 2-3x faster and can result in a 40% startup latency improvement. If the file is missing or corrupted, it will be regenerated. This regeneration happens after the application is open for a minute or so. If you use short-lived applications, the hashcache may never get generated and you will not gain any performance improvements from enabling it.

### `%PROGRAMDATA%\Packer\minst.*.bin`

- **Type**: Single-instance file lock
- **File name**: `minst.*.bin`, where `*` is a build-specific identifier.
- **File size**: 0-20 B
- **Purpose**: Used as a lock file to detect when an existing instance of the application is already running.
- **Associated configuration option**: Only used if `enforce_single_instance` is set.

# Known Incompatibilities

This document lists known incompatibilities between Theia and third party software.

## EasyAntiCheat

- The runtime config option `ntdll_antihook` must be set to `false`, otherwise it causes crashes
- The runtime config option `spoof_thread_start` must be set to `Spoof`, otherwise it causes hangs
- The runtime config option `instrumentation_callback` must be set to `TempRemove` or `UseChain`, otherwise it causes hangs or crashes

## SentinelOne EDR

- The runtime config option `instrumentation_callback` must be set to anything other than `None`

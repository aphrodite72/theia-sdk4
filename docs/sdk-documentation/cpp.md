# Theia SDK for C++

This document describes how to integrate the Theia SDK into your C++ application. For more information on the Theia SDK, see the [Theia SDK overview](./readme.md).

An online doxygen documentation for the C++ SDK can be found [here](../doxygen). This generated documentation is the canonical source of truth for the C++ SDK.

## Requirements 

- C++14 compatible compiler

## Basic Integration

Integrating the Theia SDK into your application is a two-step process. First, you must include the `theia_sdk.hpp`, found in `languages/cpp` in the Theia SDK, within your application. Secondly, you must use the `THEIA_ONCE` macro exactly once within your application. For example:

```cpp
#include "theia_sdk.hpp"

#include <cstdio>

THEIA_ONCE();

int main() {
  return 0;
}
```

This is the minimum integration required to make your application compatible with the Theia runtime.

## Interacting with the Theia runtime

The Theia SDK provides an interface for interacting with the Theia runtime. This interface is available as the `FunctionPtrs` type through `theia::GetInterface()`. Review `theia_sdk.hpp` for a current set of supported interface functions and what they do.

Example:

```cpp
#include <cstdio>

#include <theia_sdk.hpp>

THEIA_ONCE();

int main() {
  printf("exe is %s\n", theia::GetInterface()->IsProtected() ? "PROTECTED" : "NOT PROTECTED");
}
```

## Receiving SDK callbacks

Review the [SDK callbacks documentation](./callbacks.md) for information on the Theia SDK and callbacks.

Within the C++ SDK, callbacks are registered using the `THEIA_REGISTER_CALLBACK(Function, Target)` macro. `Target` can be one of the following:

- A `CrashCodeCategory`, e.g. `theia::CrashCodeCategory::AntiVM`.
- A specific crash code, e.g. `theia::CrashCode::Assert::Failed`.
- A catch-all for all crashes: `theia::CrashCode::All`.
- A `TelemetryCodeCategory`, e.g. `theia::TelemetryCodeCategory::CodeSigning`.
- A specific telemetry code, e.g. `theia::TelemetryCode::CodeSigning::RejectedUnsignedDLL`.
- A catch-all for all telemetry callbacks: `theia::TelemetryCode::All`.

`Function` must be a function that takes an `NTSTATUS`, three `uintptr_t` parameters, and returns either a `theia::SDKCallbackAction`, or `void`. If you register a function that returns `void` as a crash callback, it will be automatically detected as a crash notify callback.

The Theia SDK contains compile-time assertions and checks to ensure proper usage of the `THEIA_REGISTER_CALLBACK` macro.

You must call the `ReadyForCallbacks` function on on the Theia SDK in order to start receiving callbacks.

The following example shows various SDK callbacks implemented:

```cpp
#include <cstdio>

#include <theia_sdk.hpp>

void Assert_Failed(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
  puts("An assertion failed.");
  // By returning void, the Theia runtime generates more efficient code since it knows that
  // this crash can never be cancelled by your code. At the same time, it prevents attackers
  // from hijacking this function to always continue execution.
}
THEIA_REGISTER_CALLBACK(Assert_Failed, theia::CrashCode::Assert::Failed);

theia::SDKCallbackAction AntiVM_Failed(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
  printf("The AntiVM detection with index %d failed.\n", theia::CrashCode::GetCrashIndex(Code));
  return theia::SDKCallbackAction::Continue; // allow execution to continue, instead of crashing
}
THEIA_REGISTER_CALLBACK(AntiVM_Failed, theia::CrashCodeCategory::AntiVM);

void Catchall_CrashHandler(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
  printf("A crash was detected with category %d and index %d.\n", theia::CrashCode::GetCrashCategory(Code), theia::CrashCode::GetCrashIndex(Code));
}
THEIA_REGISTER_CALLBACK(Catchall_CrashHandler, theia::CrashCode::All);

void Telemetry_UnsignedDLL(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
  wchar_t* path = (wchar_t*) Param1;
  printf("An unsigned DLL with path %ls was prevented from loading.\n", path);
}
THEIA_REGISTER_CALLBACK(Telemetry_UnsignedDLL, theia::TelemetryCode::CodeSigning::RejectedUnsignedDLL);

THEIA_ONCE();

int main() {
  // ensure we're receiving callbacks
  theia::GetInterface()->ReadyForCallbacks();
  return 0;
}
```

## Guard pages

To improve the resilience of your application against potential attackers, we **strongly** recommend the use of guard pages in your application. Guard pages are blocks of code that must never be touched during runtime. They exist to detect attackers that attempt to dump a process by iteratively reading every page in the binary. The Theia runtime is aware which pages are guard pages, and will automatically trigger a crash if any of these pages are touched.

To insert a guard page, simply use the `THEIA_GUARD_PAGE()` macro. Due to the way this macro is designed, you can only use it once per source file. However, you are recommended to insert guard pages at the end of multiple source files. Each guard page incurs a 4kb overhead in your binary, but makes it significantly harder for attackers to dump your application (especially if they are scattered across your application).

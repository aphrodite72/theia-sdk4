# Upgrading from v1.x

This document describes how to update your application and configurations to be compatible with Theia 2. We recommend that you first step through each item in this guide to ensure that your application and configurations are ready for Theia 2. Afterwards, consider reviewing some of the new features and changed settings in Theia 2 to see if they might be appropriate for your application.

For a comprehensive list of all new changes in Theia 2.x, see [full changelist](#full-changelist). Please feel free to contact your Zero IT Lab representative if you encounter any issues during upgrading, or if you have any questions related to Theia 2 or the depreciation policy for Theia v1.x.

## SDK Changes

The majority of changes in Theia 2 can be found in the Theia SDK. Here, we have completely overhauled the `CrashCode` and SDK callback system to allow for more efficient code generation and the ability to register "telemetry callbacks": notification callbacks for certain actions taken by the Theia runtime. We have also cleaned up some of the individual crash codes, extensively documented the callback behavior for certain categories, and added compile-time checks for correct usage of the `THEIA_REGISTER_CALLBACK` macro.

With the release of v2.x, we now also provide an [automatic Doxygen-based documentation](../doxygen) for the SDK. This documentation, automatically generated from the comments in the Theia SDK, is the main source of truth for the behavior of the Theia SDK. We strongly recommend that you review it.

### Crash codes are now `NTSTATUS`es

Instead of the `theia::CrashCode` enumeration, both crash codes and telemetry codes (new in v2.x) are now [custom `NTSTATUS`](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/defining-new-ntstatus-values) values. These status codes have the `theia::STATUS_FACILITY_THEIA` facility.

The following changes are necessary when upgrading from v1.x:

- Replace all instances of `theia::CrashCode` with `NTSTATUS`.
- Replace all instances of `theia::CrashCodeHi` with `theia::CrashCodeCategory`.
- Replace all instances of `theia::CrashCode::<Category>_<Index>` with `theia::CrashCode::<Category>::<Index>`.

To obtain the category and index for a Theia NTSTATUS, the helper functions `theia::CrashCode::GetCrashCategory` and `theia::CrashCode::GetCrashIndex` are available.

The following example illustrates the changes needed to support new NTSTATUS-based crash codes:

```diff
-theia::SDKCallbackAction MyCallback(theia::CrashCode Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
+theia::SDKCallbackAction MyCallback(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
- if (Code == theia::CrashCode::AntiVM_CPUIDBackdoor) {
+ if (Code == theia::CrashCode::AntiVM::CPUIDBackdoor) {
    printf("VM detected through CPUID backdoor.\n");
  } else {
-   theia::CrashCodeHi category = static_cast<theia::CrashCodeHi>(static_cast<uint32_t>(Code) >> 16);
+   theia::CrashCodeCategory category = theia::CrashCode::GetCrashCategory(Code);
-   uint16_t index = static_cast<uint16_t>(static_cast<uint32_t>(Code) & 0xFFFF);
+   uint8_t index = theia::CrashCode::GetCrashIndex(Code);

    printf("Callback received for category %d and index %d.\n", category, index);
  }

  return theia::SDKCallbackAction::Default;
}
```

### Crash codes reorganization

To better reflect the actual behavior of the runtime, and to make it easier to handle badware crashes, we've reorganized some of the categorization of crash codes and added a few new ones.

The following changes were done:

- The `User` crash category was merged with the `Assert` category
  - The `User_Induced_Soft` crash is now `Assert::User_Induced_Soft`
  - The `User_Induced_Hard` crash is now `Assert::User_Induced_Hard`
- The `Assert_Failed` crash has been separated into several crashes, depending on the type of the assert
  - `Assert::Failed` is triggered for generic assertions
  - `Assert::FailedValue` is triggered for assertions with a value attached
  - `Assert::FailedNT` is triggered when an unexpected `NTSTATUS` value is encountered
  - `Assert::FailedWin32` is triggered when an unexpected Win32 status code is encountered
  - The diagnoser CLI tool is capable of giving more information for these assertions.
- The return value of callbacks for crashes in the `Assert` category is now explicitly ignored. `Assert` crashes will always crash the process.
  - This behavior was already partially present in v1.x, but is now consistent across the entire category.
- Callbacks for the `CodeIntegrity` category will now never be emitted. If the Theia runtime encounters a code integrity failure, it will immediately crash the process.
  - This behavior was already present in v1.x, but is now properly documented and enforced.
- Individual crashes within the `Badware` category are now badware products, instead of detection vectors.
  - This makes it easier to handle all detections of a single badware product, instead of having to register several callbacks for every possible detection method.
  - For example, the `Badware_ImageName` crash with `Param1 = BadwareProduct::IDA` in v1.x is now a `theia::CrashCode::Badware::IDA` crash.
  - We no longer expose the detection method for badware. Please contact us if you were relying on this value.
- The `CrashCodeHi::Linux` value was removed, as it was unused.

The change to the badware crash categorization is worth discussing in more detail. We observed that the majority of badware-related crash callback subscriptions were used to log or allow certain badware products. With the way that badware crashes were structed in v1.x, this meant that in order to ensure that you handled every possible detection method for the crash code, one was forced to subscribe to the entire badware category of crashes, even if you were only interested in a single badware product.

With the new structure of badware crashes in v2.x, we instead provide a unique crash code per badware product, regardless of _how_ that specific badware was detected. This allows you to easily and efficiently subscribe to callbacks for specific products without receiving callbacks for all other badware.

The following code in v1.x:

```c++
theia::SDKCallbackAction MyBadwareCallback(theia::CrashCode Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    theia::BadwareProduct product = static_cast<theia::BadwareProduct>(Param1);
    if (product == theia::BadwareProduct::XYZ) {
        // ...
    }
    return theia::SDKCallbackAction::Continue;
}
THEIA_REGISTER_CALLBACK(MyBadwareCallback, theia::CrashCodeHi::Badware);
```

Can be more efficiently represented as the following in Theia 2:

```c++
theia::SDKCallbackAction MyBadwareCallback(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    // ...
    return theia::SDKCallbackAction::Continue;
}
THEIA_REGISTER_CALLBACK(MyBadwareCallback, theia::CrashCode::Badware::XYZ);
```

Note that the Theia runtime will also generate significantly less code in the second case, as it will only need to emit callbacks for a single type of badware, instead of all possible products across all possible categories. This will improve binary size and lead to lower resource usage.

### Crash notify callbacks

The Theia SDK now supports the ability for crash "notify" callbacks. These are exactly the same as the existing crash callbacks from v1.x, except that they return `void` instead of `theia::SDKCallbackAction`. Crash notify callbacks act as if they always return `theia::SDKCallbackAction::Default`.

Whenever possible, prefer using crash notify callbacks over normal crash callbacks. Notify callbacks generate better code in the Theia runtime and they are not vulnerable to potential hijacking by an attacker. Any of your existing crash callbacks that always return `theia::SDKCallbackAction::Default` should be refactored to instead return `void`:

```diff
-theia::SDKCallbackAction MyCallback(theia::CrashCode Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
+void MyCallback(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    // ...
-   return theia::SDKCallbackAction::Default;
}
```

The `THEIA_REGISTER_CALLBACK` macro will automatically detect the type of callback from the return type of the function.

### Telemetry callback support

Theia 2 introduces a new type of callback: telemetry callbacks. Telemetry callbacks are notifications that the Theia runtime has performed some kind of non-fatal action. Unlike crash callbacks, you cannot influence the result of a telemetry callback. Just like crash callbacks, telemetry callbacks are divided into `TelemetryCodeCategory` categories, with individual telemetry events within. The function signature for telemetry callbacks is the same as for crash notify callbacks.

We currently provide a single telemetry event: `theia::TelemetryCode::CodeSigning::RejectedUnsignedDLL`. This telemetry callback is invoked when Theia is configured to enforce code signatures and an unsigned DLL was prevented from loading. Subscribing to this event is very similar to the process for subscribing to a crash callback:

```c++
void MyTelemetry(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
  PWSTR path = (PWSTR)Param1;
  printf("Unsigned DLL prevented from loading: %ls\n", path);
}
THEIA_REGISTER_CALLBACK(MyTelemetry, theia::TelemetryCode::CodeSigning::RejectedUnsignedDLL);
```

### Callback registration changes

To support crash notify callbacks and telemetry callbacks, as well as the change from `theia::CrashCode` to `NTSTATUS`, the internals of the `THEIA_REGISTER_CALLBACK` macro were entirely revamped. It is now easier to use and performs static analysis on its arguments. If provided with incorrect arguments, it will trigger a human-readable compile error with information about the issue.

The usage of the `THEIA_REGISTER_CALLBACK` remains the same, except for the syntax used for catch-all handlers. In order to allow differentiating between wildcard handlers for crash and telemetry callbacks, the value `0` is now no longer accepted. Instead, use the sentinel value `theia::CrashCode::All`:

```diff
theia::SDKCallbackAction MyCallback(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    // ...
    return theia::SDKCallbackAction::Default;
}
-THEIA_REGISTER_CALLBACK(MyCallback, 0);
+THEIA_REGISTER_CALLBACK(MyCallback, theia::CrashCode::All);
```

The following invocations for `THEIA_REGISTER_CALLBACK` are supported in v2.x:

- `THEIA_REGISTER_CALLBACK(Func, theia::CrashCode::All)`
- `THEIA_REGISTER_CALLBACK(Func, theia::CrashCodeCategory::Abc)`
- `THEIA_REGISTER_CALLBACK(Func, theia::CrashCode::Abc::Xyz)`
- `THEIA_REGISTER_CALLBACK(Func, theia::TelemetryCode::All)`
- `THEIA_REGISTER_CALLBACK(Func, theia::TelemetryCodeCategory::Abc)`
- `THEIA_REGISTER_CALLBACK(Func, theia::TelemetryCode::Abc::Xyz)`

`Func` must return `void` if used as a telemetry or crash notify callback. Otherwise, it must return `theia::SDKCallbackAction`.

### Assertion tag support

In order to aid with triaging Theia runtime crashes, the runtime now has support for optional "tags" attached to assertions. These tags stay consistent across multiple builds, even if their line number or file name changes. We recommend that you use tags to identify common crash causes between different versions of your protected executable.

Not all assertions have a tag. We will gradually attach tags to assertions that might commonly be triggered, depending on failures observed in the wild. Please reach out if you are seeing a common assertion failure across different versions without a tag.

The [diagnoser](../components/diagnoser.md) has been updated to print assertion tags whenever an assertion failure is detected.

## Configuration Changes

We've used the major version bump as an opportunity to clean up some of the configuration options and to change some of the default values. All changes to default values and file structure are documented in the remaining section.

As a reminder, we continue to keep innovating and adding new features to Theia. We recommend that you review the [runtime configuration](../configs/runtime-config.md) options to see if we've recently added a new feature that would be beneficial in your project. Please feel free to reach out if you'd like us to review your configuration.

### Changes to the module configuration

The following changes were made to default values in the module configuration schema:

- `mid_instruction_decryption_rejection` now defaults to `true` (old: `false`).
  - Testing with a wide range of executables has shown that this option is very stable and produces little to no extra overhead, while strengthening the anti-dumper protections in the Theia runtime.
  - We strongly recommend that you use the default value of `true`. If you want to preserve the old behavior, manually specify `"mid_instruction_decryption_rejection": false` in your configuration.
- `import_obfuscation` now defaults to `true` (old: `false`).
  - Enabling import obfuscation makes dumps less useful by requiring an additional import deobfuscation step, and breaks automatic import reconstruction tools such as [Scylla](https://github.com/NtQuery/Scylla).
  - On modern CPUs, the performance overhead of obfuscated imports is effectively zero. We have not observed any frame time increase in Unreal Engine as a result of import obfuscation. Only if you perform a extremely large amount of calls to imported functions in performance-critical code, you may want to consider disabling the feature by manually specifying `"import_obfuscation": false`.

We recommend that you adopt these new default values where possible.

### Changes to the runtime configuration

The following changes were made to default values in the runtime configuration schema:

- `thirdparty_license_version` is now required.
  - Distributing the appropriate third-party licenses with your binary is required in order to conform to license requirements for libraries used by Theia.
  - By making this field required, you need to consciously opt-in to a specific set of third-party licenses.
  - The only valid value for this field in v2.x is `2023-03-03`, which corresponds to the newest version supported in v1.x. If you were using the old version, `2023-02-24` (default if not specified), you will need to update the set of third-party licenses shipped with your executable. See the [documentation on third-party license versions](../third-party-licenses.md) for more information.
- `os_version.block_win7` now defaults to `true` (old: `false`).
  - Windows 7 has reached EOL and no longer receives updates.
  - While Theia will maintain compatibility with the last service pack released for Windows 7 (KB4534310), we now recommend that you drop support for the operating system entirely if possible.
  - To maintain old behavior, manually specify `"os_version": { "block_win7": false, ... }`.
- `os_version.block_win81` now defaults to `true` (old: `false`).
  - Windows 8.1 has reached EOL and no longer receives updates.
  - While Theia will maintain compatibility with the last service pack released for Windows 8.1 (KB5022352), we now recommend that you drop support for the operating system entirely if possible.
  - To maintain old behavior, manually specify `"os_version": { "block_win81": false, ... }`.
- `windowhook_sanitization` now defaults to `true` (old: `false`).
  - Prevents attackers from using malicious window hooks to inject code into your application.
  - This was disabled by default to maintain backwards compatibility.
  - This should not introduce any incompatibilities or performance issues and we strongly recommend that you keep the default value. If you encounter any issues, please contact us so that we can investigate.
- `instrumentation_callback` now defaults to `UseChain` (old: `None`).
  - Increases protection on your binary by ensuring that attackers cannot forge decryptions with very little extra overhead.
  - New default value of `UseChain` is compatible with all tested anti-cheat tools and EDRs.
  - The old default value, `None`, should be considered deprecated and no longer used, as it causes incompatibilities with SentinelOne EDR.
  - If you encounter issues with `UseChain` that disappear when set to a different value, please contact Zero IT Lab.
- `periodic_integrity_check` now defaults to `true` (old: `false`).
  - Periodically checks that the executable has not been modified in the background.
  - This was disabled by default to maintain backwards compatibility. We strongly recommend that you turn this on, as it prevents attackers from hooking your code.
  - If enabling this causes `CodeIntegrity::HashMismatch` crashes, review that you do not have self-hooking code or dependencies that might do so. Feel free to reach out to Zero IT Lab if you need help determining the source of hash mismatch crashes.
- `periodic_reencryption` now defaults to 1 page re-encryption every 16ms (old: `null`/disabled).
  - This is a new feature released in Theia v1.3, where it is disabled by default for backwards compatibility.
  - By enabling re-encryption, attackers can no longer keep playing your game to obtain more decrypted pages. This results in better protection against dumping.
  - The default value is to perform a single page re-encryption per 16ms (once per frame at 60fps), which is a very conservative value with negligible performance impact. With this default value, an average game will be entirely re-encrypted after roughly 5 minutes of gameplay.
  - The Theia runtime will prioritize pages that have not recently been used, to minimize the chance that a re-encrypted page needs to be decrypted again.
  - We have not observed measurable FPS impact for this default value on Unreal Engine games, but we recommend that you perform your own testing to verify that this assumption also holds true for your binary.

We recommend that you adopt these new default values where possible.

Additionally, the format for the `crash_handler` configuration key was changed to properly separate the fields depending on which type of crash dumper you are using. If you are using the default Theia dumper, the following example shows how the new configuration should be structured:

```diff
{
    "crash_handler": {
-       "dump_path": "abc",
-       "http": ...,
-       "support_instructions": "xyz",
+       "dumper_implementation": {
+           "type": "Gui",
+           "dump_path": "abc",
+           "http": ...,
+           "support_instructions": "xyz"
+       },
        "call_unhandled_exception_filter": false
    }
}
```

Review the [runtime config schema](../schemas/runtime-config.schema.html) for a detailed view of the expected format for `dumper_implementation`.

## Bifrost & Versioning Changes

In order to opt into the use of Theia 2, you must change your `runtime-config.json` to contain the new product version:

```diff
{
-   "product_version": "theia-1",
+   "product_version": "theia-2"
}
```

Aligned with the release of Theia 2, the Bifrost platform now allows you to pin builds to a specific major, minor, or patch version. This allows you to control which version of Theia your application gets bundled with. The following options are supported:

- `"product_version": "theia-2"`: Build with the newest version of Theia 2.
- `"product_version": "theia-2.n"`: Build with the newest patch version of Theia 2.n.
- `"product_version": "theia-2.n.n"`: Build with the specific Theia version 2.n.n.

Theia adheres to the [semantic versioning specification](https://semver.org/). In particular, this means that all releases within Theia 2 are backwards compatible. Because of this, we strongly recommend that you keep using the `theia-2` product version to ensure that you always have the latest version without having to worry about backwards-incompatible changes.

## Tooling Changes

As part of the v2.x release, we have made minor changes to the `packer` and `diagnoser` tools. Their general usage remains the same.

### Packer changes

- The Theia packer now requires the presence of Theia SDK v2.x in order to function. Binaries linked with a Theia SDK intended for v1.x will be rejected.
- The Theia packer now puts generated code in buffers reserved by the Theia SDK. This means that the output packed binary is the same size as the input, and that WinDbg will automatically find debug symbols.

### Diagnoser changes

The Theia [diagnoser tool](../components/diagnoser.md) has been expanded with support for the new separation of `Assert` crash codes. If a `NT` or `Win32` assertion is hit, it will additionally attempt to print out the message and name associated with the encountered error. The diagnoser now also has support for assertion tags.

An example output of the updated diagnoser is the following:

```plain
 INFO diagnoser: This dump is of a process with a Theia instance loaded:
 INFO diagnoser: - Bifrost job ID: ffeeddccbbaa99887766554433221100
 INFO diagnoser: - Process uptime: 2s
 INFO diagnoser:
 INFO diagnoser: Detected a Theia soft crash at 0x7ffe9d200000.
 INFO diagnoser: The process crashed with status 0xe0670103.
 INFO diagnoser: This represents an assertion failure within the Theia runtime.
 INFO diagnoser:
 INFO diagnoser: Assertion failed at 43e28b05dd7d4d00:429.
 INFO diagnoser: Assertion tag: 61626364 (abcd)
 INFO diagnoser:
 INFO diagnoser: The Theia runtime encountered an unexpected NTSTATUS code: ffffffffc0000022
 INFO diagnoser: STATUS_ACCESS_DENIED: {Access Denied} A process has requested access to an object, but has not been granted those access rights.
 INFO diagnoser:
 INFO diagnoser: If you believe this to be a Theia bug and not a result of faulty, misconfigured, or resource-starved hardware, please contact Zero IT with your dump file.
```

If you rely on the output of diagnoser, please review that this new output format works with your tooling.

## Full changelist

The following is a full changelist of all changes made between v1.x and v2.x. It omits internal changes, and as a result should not be seen as a complete changelist. Instead, we provide this list as a quick overview for you to confirm that you are aware of all changes and have updated your application and configurations accordingly.

- General:
  - Update several dependencies and compiler version.
- SDK:
  - Crash codes are now NTSTATUSes.
  - CrashCodeHi renamed to CrashCodeCategory.
  - CrashCode enum converted to theia::CrashCode::category::value constants.
  - Add support for crash notify callbacks that return void.
  - Inverted badware product and crash code relation, such that the product is the main index of the crash code.
  - Reserve some space for packer-related data in text/rdata/data sections.
  - Reword guarantees and behavior for `Encrypt` interface method.
  - Export Theia major SDK and interface version to packer.
  - Revamp callback registration and add proper static analysis.
  - Diagnostic codes and callback support.
  - Remove old unsupported 2023-02-24 third-party license version.
- Runtime:
  - CodeIntegrity crashes no longer invoke any crash callbacks.
  - Assert crashes now properly ignore return value of crash callbacks.
  - Randomize upper part of crash code to prevent hard crashes from being consistent between runs.
  - Dispatch dll prevented loading diagnostic callbacks.
  - Split up assertions into Failed, FailedValue, FailedNT, FailedWin32.
  - Add support for assertion tags to identify the same assertion between builds.
  - Change encryption key for hard crashes.
- Packer:
  - Derive instance hash from both the input and the module config to ensure it is unique for everything that contributes to the output.
  - Add support for parsing new SDK, drop support for v1.x SDK.
  - Insert packer changes into the space reserved by the SDK to avoid changing SizeOfImage.
- Diagnoser:
  - Support new hard crash encryption key.
  - Support assertion tags and show them when available.
  - Ignore upper part of crash code with random information.
  - If the crash was a Win32 or NTSTATUS assertion failure, print error code and description if available.
- Configurations:
  - Module config
    - mid_instruction_decryption_rejection: default value changed from false to true
    - import_obfuscation: default value changed from false to true
  - Runtime config
    - thirdparty_license_version: Is now required
    - os_version.block_win7: default value changed from false to true
    - os_version.block_win81: default value changed from false to true
    - windowhook_sanitization: default value changed from false to true
    - instrumentation_callback: default value changed from None to UseChain
    - periodic_integrity_check: default value changed from false to true

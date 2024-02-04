# Process integrity options

The Theia runtime offers a set of "process integrity" options that attempt to prevent the injection of rogue code into the process. In particular, these settings look for executable code pages that do not directly belong to a loaded image (e.g. the executable or any of its loaded DLL dependencies). The presence of such code pages generally either indicates that the process has performed [just-in-time compilation](https://en.wikipedia.org/wiki/Just-in-time_compilation) or that code has been injected into the process from outside. In particular, modules that have been "manual mapped" into a process (bypassing the native Windows image loading functionalities, which Theia uses to enforce code signatures if enabled) often allocate large pages of executable memory.

While the process integrity options are effective at stopping a wide range of code execution and injection attacks, their settings need to be carefully evaluated. **Unless configured properly, the process integrity options may also prevent "legitimate" software, such as overlays or gameplay recording software, from injecting into the game!** While the Theia team has evaluated the impact of these settings on common software suites, we cannot guarantee their compatibility with every potential application that may decide to inject into your application.

## Available options

The process integrity options offer the following sub-options:

- `floating_executable_memory`: Describes what Theia should do when it detects executable memory regions in the process that do not belong to a legitimately loaded (and signature checked, if enabled) module.

Option can be configured using the following values:

- `Allow` (default): Do not perform any detections.
- `AllowSmall`: Query the size of the executable memory region. If it is "small" (`size <= 0x10000`), do nothing. If it is a relatively large memory segment (often an indicator that a large module has injected itself into the process), attempt to strip executable permissions from memory region.
- `Deny`: Always attempt to strip executable permissions from memory region.

## Recommended settings

Generally, we recommend the following settings. However, depending on the set of features used by your applications, you may need to review the [compatibility list](#compatibility-with-other-software) and update your settings accordingly.

- `floating_executable_memory`
  - Value: `AllowSmall`
  - Reasoning: Most legitimate injecting software (e.g. Discord, Steam overlay, OBS) use small memory segments to detour functions and to inject themselves into the process. We have generally not observed any large executable allocations from legitimate software. Common malicious DLL injectors that use the "manual mapping" approach tend to allocate large sections, which will be blocked by this value.

## Compatibility with other software

This is an incomplete list of common software and libraries whose workings may be impacted by the process integrity options. We make no guarantees about the accuracy of this list, and it should only be used as a guideline for determining which options are suitable for your product. You should always perform tests to ensure that your application stays compatible with any third-party software and libraries that you wish to support.

**DX Software Rasterization/Windows Advanced Rasterization Platform (WARP)**  
- `floating_executable_memory`
  - Supported values: `AllowSmall` (recommended), `Allow`
  - Reason: The software rasterizer performs just-in-time compilation of shaders to SIMD code blocks. We have not observed these allocations exceeding a size of 0x10000.

**IMPORTANT NOTE**: WARP will be automatically enabled by Windows if your application uses DX8 or newer and the PC has no (working) hardware graphics acceleration. Unless your application is a daemon, command-line application, or uses Win32 components only, you will almost certainly need to consider the usage of WARP in your application.

**CEF/Chromium/V8 (JIT)**
- `floating_executable_memory`
  - Supported values: `AllowSmall` (recommended), `Allow`
  - Reason: The V8 JavaScript engine performs just-in-time compilation using RWX pages. We have not observed these allocations exceeding a size of 0x10000.

**LuaJIT**
- `floating_executable_memory`
  - Supported values: `AllowSmall` (recommended), `Allow`
  - Reason: LuaJIT performs just-in-time compilation using RX pages. In its default configuration, these pages are exactly 0x10000 in size and therefore will be allowed using the `AllowSmall` configuration. Note that this value can be adjusted using the `JIT_P_sizemcode` setting, so it is recommended that you review whether your code or any dependencies adjusts this value.

**Steam (overlay)**
- `floating_executable_memory`
  - Supported values: `AllowSmall` (recommended), `Allow`
  - Reason: Steam allocates executable memory to hook certain Windows library functions. We have not observed these allocations exceeding a size of 0x10000.

**Discord (overlay)**
- `floating_executable_memory`
  - Supported values: `AllowSmall` (recommended), `Allow`
  - Reason: Discord allocates executable memory to hook certain Windows library functions. We have not observed these allocations exceeding a size of 0x10000.

**OBS (Game Capture)**
- `floating_executable_memory`
  - Supported values: `AllowSmall` (recommended), `Allow`
  - Reason: OBS allocates executable memory to hook certain rendering functions. We have not observed these allocations exceeding a size of 0x10000.
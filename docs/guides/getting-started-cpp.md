# Getting started in C++

This page describes how to get started with Theia and the Theia SDK for your application in C++. We go through all steps needed to integrate your application with the Theia SDK, encrypting the resulting binary, and obtaining a dedicated Theia runtime library capable of protecting the packed result.

This page functions as a **quickstart guide only**. The reader is encouraged to review all other pages and documentation for more information about all the features offered by Theia.

## Integrating the SDK

We begin by integrating the [C++ Theia SDK](../sdk-documentation/cpp.md) into your application. The SDK allows you to communicate with the Theia runtime, to receive notifications on crashes and blocked actions, and to ensure that your binary can be packed by the Theia packer.

Begin by downloading `theia_sdk.hpp` from `languages/cpp` in the SDK repository, and adding it to your project. It is designed to work with any C++14-compatible compiler on Windows.

We'll register a [crash callback](../sdk-documentation/callbacks.md) for the `Badware` category. This will invoke our callback whenever the Theia runtime detects prohibited software, such as debuggers or hacking tools. For our example, we'll simply alert the user that the impending crash was caused by bad software, and that they should close whatever software might be responsible.

```cpp
#include "theia_sdk.hpp"

// This is ALWAYS required, and must be invoked exactly once.
THEIA_ONCE();

// This callback will be invoked whenever Theia raises a badware-related crash.
void BadwareDetected(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
  MessageBoxW(
    NULL,
    L"Cheating or reverse-engineering software detected. This application does not allow such software and will now close. Please close the offending software and restart the application.",
    L"Incompatible Software Detected",
    MB_OK | MB_ICONERROR
  );

  // since this is a void-callback, the runtime will perform the default option
  // for badware-related violations, the default option is to trigger a crash
}
THEIA_REGISTER_CALLBACK(BadwareDetected, theia::CrashCodeCategory::Badware);
```

To ensure that we will receive the crash callback, we need to ensure that we signal to the Theia runtime that we are ready to receive callbacks. Place the following call somewhere in your initialization logic, so that it is invoked whenever you're ready to receive callbacks:

```cpp
theia::GetInterface()->ReadyForCallbacks();
```

Compile and run your application. It should still work normally even when not protected by Theia.

## Making a Theia configuration

Before your application can be packed, we must configure the Theia packer and runtime. For this, we use two different JSON files, usually named `module-config.json` and `runtime-config.json`.

### The module configuration

The module configuration is used to configure certain Theia options on a _module_ level. Here, a module refers to a single .exe or .dll being protected. Since Theia supports protecting more than one module at a time (e.g. you are able to protect both your main executable, and a library dependency), all settings in the module configuration only apply to the specific module for which you use the configuration.

For this guide, put the following content in a file called `module-config.json`. This is a set of sane values perfect for getting started with Theia. For more information on the available options, see [the documentation for module configuration](../configs/module-config.md).

```json
{
  "preloader_name": "preloader.dll",
  "tracing_mode": "TraceGlobal",
  "ept_hook_detection": true,
  "encrypt_section_padding": false,
  "mid_instruction_decryption_rejection": true,
  "import_obfuscation": true
}
```

### The runtime configuration

The runtime configuration is used to configure Theia options that are global to the entire runtime generated for your protected application. If you protect more than one module with your runtime, settings in the runtime configuration will apply to the entire process.

For this guide, we'll use the following `runtime-config.json`. A full listing of all available options can be found [here](../configs/runtime-config.md).

```json
{
  "product-version": "theia-2",
  "runtime_name": "runtime.dll",
  "thirdparty_license_version": "2023-03-03",
  "crash_handler": {
    "dumper_implementation": {
      "type": "Gui",
      "support_instructions": "Oops! We ran into a problem and crashed. Please help us out by creating a dump and sending it to us at support@mycompany.com."
    },
    "call_unhandled_exception_filter": false
  },
  "signature_check": false,
  "antidebug": true,
  "antiemulation": true,
  "antihv": true,
  "antivm": true,
  "badware": {
    "allow_all": false,
    "debuggers": false,
    "memhack_tools": true,
    "lldb": true,
    "procmon": true,
    "wireshark": true,
    "ida": true,
    "cffexplorer": true,
    "pebear": true,
    "winobjex": true,
    "injectors": true,
    "unknowncheats_generic": true,
    "dtrace": true,
    "processhacker": true,
    "procexp": true,
    "npcap": false
  }
}
```

**THIS IS NOT A SAFE CONFIGURATION FOR PRODUCTION USE!** This config includes some settings that make it easier to get started with Theia but that should really be turned off when using Theia in production. For example, we turn off `signature_check` to avoid having to code-sign all of the binaries, and we turn off badware detection for `debuggers` and `npcap` to avoid accidental crashes for software likely to be running on a developer's machine.

## Packing the executable

Now that we have integrated the SDK and prepared a configuration, the next step on our list is to "pack" your executable using the [Theia packer](../components/packer.md). The packing process will encrypt most parts of your executable, remove all imports and other information beneficial to attackers, and prepare the binary for use with the Theia runtime. Packing results in a "metadata file", which describes how the binary was packed (e.g. which keys were used for encrypting), and the packed binary itself. You submit the metadata file to Zero IT Lab to generate a unique [Theia runtime](../components/runtime.md) for your binary.

The Theia packer is a command-line application. If you do not have access to it, please contact your Zero IT Lab representative.

To pack your application, execute the following command in the terminal:

```sh
$ packer.exe --module-config ./module-config.json --metadata metadata.bin --output packed.exe --pdb path/to/your.pdb path/to/your.exe
```

If everything was configured properly, this command should succeed and print out some statistics on the transformations it did. You should now also have a `metadata.bin` and `packed.exe` in the current directory. If you try running the packed executable, you'll notice that nothing happens. This is because we need to use the `metadata.bin` to build a dedicated runtime.

## Obtaining a runtime

We can use the `metadata.bin` generated by the packer to obtain a custom-built Theia runtime by submitting it to the "Bifrost" platform. To do this, we'll use the `agent` binary provided to you by your Zero IT Lab representative:

```sh
$ agent.exe --config-file ./runtime-config.json --main-meta metadata.bin --print-notices
```

The Bifrost agent selects the specific Theia version to use for the runtime through the `product-version` field inside the runtime configuration. We've specified this to be `theia-2`, which will automatically use the newest version of Theia 2.

After a few minutes, a custom-built Theia runtime will be downloaded into the current directory, as `runtime.dll` and `preloader.dll`. The packed binary requires both.

## Running and shipping the packed executable

Place the `runtime.dll` and `preloader.dll` next to the `packed.exe` output from the packer. Running `packed.exe` should now result in the exact same binary as your original, except now protected by Theia! Now try launching a "badware" program such as Cheat Engine, and you'll notice that the application invokes our crash callback.

That's it! Your application is now protected by Theia, without the actual application or your debug symbols ever leaving your computer.

Want to ship the resulting binary? You'll only need to include the packed executable, the runtime, and the preloader. You'll also need to ensure that you properly attribute the [open-source libraries](../third-party-licenses.md) that Theia uses somewhere where users of your application can access it.

Do **not** ship your PDB, original executable, `metadata.bin`, or the `packer.exe` or `agent.exe` executables! They're for development and packing only, and exposing any of them to the public will likely result in a significant or total loss of protection.

## What's next?

Congratulations on protecting your first binary using Theia! Continue your journey by:

- Reviewing the available [configuration options](../configs/readme.md) for Theia, and enabling those appropriate for your application.
- Learning about the [major features](../features/readme.md) available in Theia, and how they help protect your executable from attackers.
- Reading how the [diagnoser](../components/diagnoser.md) can help you diagnose crashes generated by Theia.
- Going into the intricacies of [SDK callbacks](../sdk-documentation/callbacks.md) and other important parts of the Theia SDK.
- Once done with integration, reviewing the [pre-release checklist](../knowledge-base/prerelease-checklist.md).

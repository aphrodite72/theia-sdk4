# Frequently Asked Questions

This page lists several frequently asked questions and their answers. Should you have a question not answered by this page, and not found elsewhere in the Theia documentation, please feel free to reach out to your Zero IT Lab representative.

### Where do I obtain the Theia packer, diagnoser, and Bifrost agent?

These binaries do not follow the release cadence of the Theia runtime and will be provided to you by your Zero IT Lab representative. Please take care to limit access to these binaries to those that need them, as to avoid accidental publishing or leaking of the binaries.

### Should I run the packing process as part of CI? How long does it take to pack and generate a runtime?

Yes! We strongly recommend that you set up the Theia packing process as part of your CI pipeline. Since Theia makes it harder to debug your application, we recommend that you apply it to QA, staging, and production builds. For day-to-day development of your application, packing your application is not necessary.

Running the Theia packer can take up to 15s, depending on the size of your input binary and PDB. The Theia runtime generation through the Bifrost platform generally takes between 2-4 minutes. If your Bifrost build consistently takes longer than that, please reach out to Zero IT Lab.

### How do I debug crashes in protected executables?

A protected executable can crash in two different ways: either the crash is caused by Theia (due to a detection, an integrity check failing, a bug in Theia, etc.), or the protected application itself has a bug. To distinguish between the two, you can use the [diagnoser](../components/diagnoser.md) tool provided for this purpose. If the crash is caused by Theia, the diagnoser will provide information on the crash code and/or potential causes of the crash.

If the crash is caused by your own code, you can proceed with debugging as you normally would. Offsets within the protected executable are the same as in the unprotected executable (with the notable exception of [stolen functions](../features/function-stealing.md)), and WinDbg is able to load the PDB of the unprotected binary when debugging the protected binary.

If you are having difficulty identifying the cause of a crash, please feel free to reach out to your Zero IT Lab representative and we'll be happy to help.

### What's the difference between `{}` and `null` in configuration files?

The Theia packer and runtime recursively applies the default values from the configuration schemas when a value is not present. This means that the notation `{}` applies the default values for all subkeys defined in the configuration schema. The `null` value means to disable the feature entirely. In other words, `{}` enables a feature with all default settings, `null` disables it.

To illustrate, the [recommended module configuration](../configs/module-config.md) contains the following line:

```json
    "nanomites": {},
```

At the time of writing, the Theia packer will expand this to:

```json
    "nanomites": {
        "removal_threshold": 100,
        "max_removal_ratio": 0.3
    }
```

### Should I expect more crashes when I protect my application using Theia?

Integrating Theia into your application will have an effect on the number of crashes experienced by your users. However, this increase has a few reasons and isn't always a bad thing. Let us elaborate a bit.

Generally, integrating Theia into your application introduces the following "new" classes of crashes:

1. Crashes as a result of unauthorized software detections, virtual machines, or other unsupported configurations.
2. Crashes as a result of incompatible third-party software, such as gaming overlays, anti-viruses or enterprise EDRs.
3. Crashes that previously went unreported/undetected, but are now caught by Theia's integrated crash dumper.
4. Crashes that are a result of faulty or overloaded hardware, which trigger integrity checks or timeouts.

The first category is entirely controlled by you and crashes from this category are intentional. By tuning your [runtime configuration](../configs/runtime-config.md), you are able to control whether Theia performs detections for classes of badware, virtual machines, insecure configurations, etc. When such a check is triggered, you _want_ to trigger a crash. Such crashes still might be reported by users.

The second category is a bug in Theia and we are constantly at work to resolve such incompatibilities. If you encounter multiple different users with similar crash reports, please contact Zero IT Lab so we can help you diagnose possible incompatibilities.

The third category are not caused by the Theia integration, but are a side-effect of Theia having better crash reporting. One such example is crashes during shutdown of your application, which are usually ignored by Windows but will still be caught and reported by Theia. We recommend that you try to reproduce these crashes without Theia under a debugger, and to contact us if you encounter a crash that is only reproducible in a protected binary.

The final category is a result of Theia's integrity checks and anti-suspending facilities. Faulty hardware as a result of bad overclocking, a faulty PSU, electrostatic interference, or other causes can result in computations yielding wrong values. Similarly, resource-starved programs as a result of underpowered or overloaded hardware can result in game threads not being scheduled for an extended period of time. Where previously such stutters and faulty computations would either go unnoticed or result in silent corruption, they are now detected by Theia's integrity checks. Crashes that fall in this category can often be easily identified because they are nonsensical (e.g. the CPU raised an access violation on an instruction that does not access memory). Whether Theia is present or not makes no difference to the end user, as the software is very likely to crash when such malfunctions happen anyway.

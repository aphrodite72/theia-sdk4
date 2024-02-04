# Runtime configuration

The runtime configuration is a [JSON configuration file](./readme.md) that configures the behavior of the [Theia runtime](../components/runtime.md). Settings configured in this file apply to the entire process, and all protected modules within it.

> **Note:** The canonical reference for the runtime configuration is the JSON schema available at `schemas/runtime-config.schema.json` and visible online in the [automatically generated HTML reference](../schemas/runtime-config.schema.html). This page elaborates on several options, but does not list all of them. The reader is strongly advised to treat this page only as additional information on some of the options, and not as a complete reference of all available options.

The runtime config file is a JSON file that must adhere to the appropriate JSON schema, available in `schemas/runtime-config.schema.json`. This JSON schema documents all available options for the runtime configuration, as well as the defaults applied when a given option is omitted. The reader is recommended to consult the [automatically generated HTML reference](../schemas/runtime-config.schema.html) for an up-to-date list of all available options. The remainder of this document elaborates on some of the options in the document.

---

For full protection, we recommend the following configuration. Review the aforementioned reference for information on each of the options.

```json
{
  "$schema": "path-to-sdk/schemas/runtime-config.schema.json",
  "product-version": "theia-2",
  "crash_handler": {
    "dumper_implementation": {
      "type": "Gui",
      "support_instructions": "To contact our customer support, go to https://your-site-here.com/support and attach the crash dump file to your ticket."
    },
    "call_unhandled_exception_filter": true
  },
  "os_version": {
    "block_win7": false,
    "block_win81": false,
    "block_wine": false
  },
  "spoof_thread_start": "SpoofKillExternal",
  "windowhook_sanitization": true,
  "vulnerable_driver": "Detect",
  "instrumentation_callback": "UseChain",
  "debug_register_usage": {}
}
```

## Configuration toggles and crash callbacks

Some options exposed in the runtime configuration can theoretically also be implemented using Theia's [crash callback](/readme.md#receiving-crash-callbacks) system. For example, instead of setting `antivm` to `false`, one could instead always return `SDKCallbackAction::Continue` when receiving an `CrashCodeCategory::AntiVM` crash.

If possible, it is highly recommended that you **prefer runtime configurations over unconditional `Continue`s in crash callbacks**. Disabling certain classes of anti-tamper/badware/integrity checks in the runtime configuration will allow the built runtime to eliminate the code needed for these integrity checks entirely, resulting in a smaller runtime and less time wasted on performing ignored detections. If you would like to statically disable a detection for which there does not currently exist a runtime configuration toggle, feel free to reach out to your Zero IT Lab representative to discuss potential options.

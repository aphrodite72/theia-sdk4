# Module configuration

The module configuration is a [JSON configuration file](./readme.md) that configures an individual protected module. While most of the options in the module configuration apply to the packer, some options may also control the runtime behavior.

> **Note:** The canonical reference for the module configuration is the JSON schema available at `schemas/module-config.schema.json` and visible online in the [automatically generated HTML reference](../schemas/module-config.schema.html). This page elaborates on several options, but does not list all of them. The reader is strongly advised to treat this page only as additional information on some of the options, and not as a complete reference of all available options.

The packer expects a `--module-config path/to/module-config.json` argument for each protected module. This config describes exactly how the packer should transform the input module. The config file is a JSON file that must adhere to the appropriate JSON schema, available in `schemas/module-config.schema.json`.

The given JSON schema documents all available options for the module configuration, as well as the defaults applied when a given option is omitted. We provide an [automatically generated HTML reference](../schemas/module-config.schema.html) for an up-to-date list of all available options. The remainder of this document elaborates on some of the options in the configuration.

## Tracing modes

The Theia packer attempts to discover instructions by tracing all reachable instructions. This tracing is aided by the function ranges defined in the input PDB and the runtime exception data. In particular, the Theia packer will use these ranges as seed data for the tracing algorithm, which will process them to discover all possible statically traceable paths.

The `tracing_mode` configuration key determines which strategy is used for this tracing. The following values are valid:

- `TraceLocal` (default): For every function range defined in a PDB or runtime exception data, follow all reachable instructions within the range. Ignore any jumps that go outside the range. Generally accurate and detects the majority of instructions without false positives.
- `TraceGlobal`: Similar to TraceLocal, but instead keeps tracing executions as long as they fall within any function range. Generally produces more complete tracing than TraceLocal, but makes the packing process slower.
- `TrustDefinition`: Do not perform tracing, and instead fully trust the ranges defined in the PDB and exception data. This will guarantee that all instructions are defined, even those only referenced indirectly, but will perform wrong results if any jump tables are present. If you are able to disable jump tables for the entirety of your application (including dependencies), this will yield the best results.

Note that the choice of `tracing_mode` only influences the _packing time_, not the runtime performance of the packed application. If you are able to, we strongly recommend using a slower but more accurate method such as `TraceGlobal`.

## Load encryption

The Theia packer automatically discovers which parts of your input module can be safely encrypted at runtime. For those that cannot be continuously encrypted at runtime, it will instead "load encrypt" those pages. This encrypts them on disk and decrypts them immediately on startup. While this improves the security of the binary, for processes that need to start quickly the extra startup time may not be worth the marginal extra protection.

Depending on the size of your binary, the decryption of load-encrypted pages can account for roughly 10% of the startup time of your binary. If your process requires fast startup time, such as modules that spawn lots of child processes (e.g. Chromium/CEF), you may want to consider disabling load-encryption.

## Function stealing

[Function stealing](../features/function-stealing.md) is a feature that allows the packer to extract certain eligible functions from your binary and insert them in the built `runtime.dll` instead. This makes it harder for an attacker to construct a "Theia-less" binary. Review the [dedicated page](../features/function-stealing.md) for more information on function stealing.

You can control the behavior of function stealing in the packer by defining options in the `function_stealing` object. The [JSON schema](../schemas/module-config.schema.html) lists the full set of available options.

Note that the `always_steal` and `never_steal` options contain regular expressions that apply to the function name as defined in the input PDB. This format is generally a simplified version of the symbol, e.g. `Class::functionName`.

If a `max_stolen_functions` value is set, the set of functions that will be stolen is randomly selected. The seed used for randomization is derived from the input binary, so while a packer invocation is deterministic, different functions may be selected for stealing on subsequent builds. It is recommended that you review proper functioning of the binary with `max_stolen_functions` set to `-1` (e.g. all functions stolen). If you encounter any bugs caused by function stealing, please contact us.

An example configuration for function-stealing:

```json
{
  "$schema": "../../schemas/module-config.schema.json",
  "function_stealing": {
    "always_steal": ["MyReflection::get.*"],
    "never_steal": ["^someHotFunctionWhereEveryMicrosecondCounts$"],
    "max_stolen_functions": -1,
    "steal_unwind_data": true
  }
}
```

By default, the packer only notifies you of the amount of functions stolen. If you would like to see the full list of functions stolen, you can enable verbose logging by setting the `THEIA_LOG` environment variable to `packer=debug` (the `=` is part of the value). This may also be helpful for determining the regular expression format needed to match certain functions, as verbose logging will print out the name of the function as embedded in the PDB.

## Nanomites

[Nanomites](../features/nanomites.md) are an additional protection that replace conditional jumps in your binary with calls to the Theia runtime instead. While they offer excellent anti-reversing protection, they come with significant performance penalties.

To somewhat avoid these excessive performance penalties, the Theia runtime will automatically restore nanomites that are hit often enough. The `removal_threshold` configuration option can be used to determine what counts as "often enough". In particular, the Theia runtime performs a periodic task that automatically removes "hot" nanomites. In order for a nanomite to be removed, it must be hit enough times within the interval of the periodic task to be removed. We do not specify an exact interval time for the periodic removal task, but it is guaranteed to be less than one second.

Do note that due to the periodic nature of the removal task, even hot nanomites will still give performance drawbacks until they are removed. In games, this will often manifest as an initial drop in frames as new rendering code gets executed. Such a drop will generally happen during, or directly after, the initial game loading screen. It is recommended that you review for yourself on several classes of hardware whether this performance drawback is acceptable.

In order to prevent an possible attack where an attacker will intentionally trigger nanomites to remove them, the Theia runtime maintains a maximum ratio of nanomites that may be removed at once. This ratio can be controlled with the `max_removal_ratio` setting. As a guideline, during testing we observed that the set of hot nanomites within the UE4 ShooterGame example encompass roughly 5-10% of the total amount of nanomites in the binary. Setting the nanomite removal ratio to zero will disable nanomite removal for the module entirely. This is useful for modules that do not contain performance-sensitive code.

## Mid-instruction decryption rejection

Mid-instruction decryption rejection is an additional security feature that will prevent reads and executes originating from or targeting bytes that are in the middle of an instruction. While disabled for backwards compatibility reasons, it is strongly recommended to enable this feature as it has negligible overhead and prevents attackers from using "gadgets" to dump pages. For more information, see the [section on mid-instruction decryption rejection in the documentation on full-read tracking](../features/full-read-tracking.md#mid-instruction-decryption-rejection).

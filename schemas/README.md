# Theia Configuration Schemas

The [JSON Schemas](https://json-schema.org/) in this folder describe the acceptable configuration values for the Theia
packer and runtime. You can use these schemas with [Hyperjump JSV](https://json-schema.hyperjump.io/) (online) or any of
the other [validators](https://json-schema.org/implementations.html#validators) to validate whether your configuration
file is valid.

# **Please see the [relevant page in the documentation book](../docs/configs/readme.md) for more information.**

Many modern editors also have first-class support for linting and autocompletion using a JSON schema. To do so, add the `$schema` property to the root of the configuration file, pointing to the appropriate schema path. For example, the following file will automatically lint in Visual Studio Code and most JetBrains IDEs:

```json
{
  "$schema": "/C:/path/to/module-config.schema.json",
  "tracing_mode": "A"
}
```

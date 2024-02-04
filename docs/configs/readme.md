# Theia Configuration

The Theia packer and runtime can be configured through JSON configuration files. We provide [JSON Schema](https://json-schema.org/) specifications for all available configurations within the `schemas` folder of the SDK repository. You can use these schemas with [Hyperjump JSV](https://json-schema.hyperjump.io/) (online) or any of the other [validators](https://json-schema.org/implementations.html#validators) to validate whether your configuration file is valid.

Many modern editors also have first-class support for linting and autocompletion using a JSON schema. To do so, add the `$schema` property to the root of the configuration file, pointing to the appropriate schema path. For example, the following file will automatically lint in Visual Studio Code and most JetBrains IDEs:

```json
{
    "$schema": "/C:/path/to/module-config.schema.json",
    "tracing_mode": "A"
}
```

Additionally, we provide rendered versions of these schemas in our online documentation. The respective sub-pages of this section provide links to these rendered views.

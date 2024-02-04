# Theia SDK

Welcome to the Theia SDK! This repository contains the Theia SDK source files for supported programming languages, as well as documentation and schema files for Theia.

This is the branch for **Theia 2.x.**. Please ensure that you are on the appropriate branch corresponding to the Theia version you are using.

We strongly recommend that you read the [online Theia SDK documentation](https://zeroitlab.gitlab.io/theia-sdk/v2) before getting started with Theia.

This repository is structured as follows:

- `docs`: Documentation for the Theia SDK. Best viewed [online](https://zeroitlab.gitlab.io/theia-sdk/v2).
- `dumper`: Libraries for implementing a custom crash dump UI. [Documentation](https://zeroitlab.gitlab.io/theia-sdk/v2/dumper-sdk.html).
- `languages`: The Theia SDK for all supported languages:
  - `languages/cpp`: The Theia SDK for C++14 and newer.
- `schemas`: A set of [JSON schemas](https://json-schema.org/) describing the configuration options supported by Theia.
  - `schemas/runtime-config.schema.json`: The runtime configuration schema. A [rendered version](https://zeroitlab.gitlab.io/theia-sdk/schemas/runtime-config.schema.html) is available online.
  - `schemas/module-config.schema.json`: The module configuration schema. A [rendered version](https://zeroitlab.gitlab.io/theia-sdk/schemas/module-config.schema.html) is available online.
- `thirdparty-licenses`: A directory of available versions of license attributions required when using Theia.

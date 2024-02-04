# Introduction

<img src="./assets/img/theia_text.svg" width="200">

Welcome to the documentation for Theia and the Theia SDK! This set of documents contains instructions on how to integrate the Theia SDK into your application, details on how to interact with the Theia runtime, and detailed information of available Theia features and their associated configuration.

The Theia SDK is available [on GitLab](https://gitlab.com/zeroitlab/theia-sdk) and consists of the following parts:

- `docs`
  - A [mdBook](https://rust-lang.github.io/mdBook/) "book" containing documentation on how to use the Theia SDK and elaborating on various features.
  - We recommend that you read through this at least once when you first start integrating Theia into your application.
  - You're currently reading this!
- `schemas`
  - A set of [JSON schema](https://json-schema.org/) specifications for the configuration that Theia accepts.
  - We automatically generate a user-readable website for these schemas, accessible at the following paths:
    - [module-config.schema.json](./schemas/module-config.schema.html)
    - [runtime-config.schema.json](./schemas/runtime-config.schema.html)
- `thirdparty-licenses`
  - A set of available third-party license versions, allowing you to easily comply with the license requirements of libraries used by Theia.
- `languages`
  - The actual Theia SDK, needed to integrate Theia into your application.
  - These are best viewed on GitLab or in your local code editor.
- `dumper`
  - An additional SDK that can be used to write custom crash dumpers for your application.

We strive to document everything related to the Theia SDK and the Theia runtime in this repository and book. If you have any questions, comments, or feature requests, please feel free to reach out to your Zero IT Lab representative.

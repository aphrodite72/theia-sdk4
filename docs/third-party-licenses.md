# Theia Third Party Licenses

The Theia runtime uses open-source code from several projects that require attribution bundled with the application. When using the Theia runtime in your application, you will need to distribute a set of license agreements corresponding to the libraries used in that version of the Theia runtime.

As the Theia runtime develops, we may occasionally add or remove libraries that require attribution. To ensure that you are not suddenly in violation of license agreements, you must consciously indicate in your [runtime configuration](./configs/runtime-config.md) which specific version of the Theia third-party licenses you want to use. Depending on this option, certain features or optimizations in the Theia runtime may not be available.

**You MUST carefully ensure that you ship the correct set of third-party notices corresponding to the version of Theia you use. Failure to do so may result in violation of copyright law.**

Theia third-party license versions are identified by the date on which they were introduced. Within this folder are subfolders for each available license version, containing the exact set of licenses in use for that specific version. The licenses are available as a structured XML format, as well as a formatted plaintext version. The remainder of this document documents the differences between each version, as well as what features or optimizations they affect.

## Available versions

The following is a changelog of available versions. Newer versions are listed at the top, and generally contain all features and optimizations from prior versions. It is recommended that you always use the most recent version available, and that you upgrade to the most recent version in a timely manner.

- Version `2023-03-03`
  - **Theia changes:** Enables Theia to use a faster hashing implementation for code signature checking, resulting in an average 20% speedup in signature checking performance.
  - **License changes:** Added the [Intel ISA-L Crypto](https://github.com/intel/isa-l_crypto) library under a BSD-3 license.
- ~~Version `2023-02-24`~~ (**not available in v2.x**)
  - **Theia changes:** Initial release.
  - **License changes:** Initial release.

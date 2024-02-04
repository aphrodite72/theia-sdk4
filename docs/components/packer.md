# Theia Packer

The Theia packer is responsible for taking your application and encrypting its contents. It outputs a protected binary, as well as a metadata file describing the transformations made to your input binary. You submit this metadata file to Zero IT Lab's "Bifrost" platform, which will automatically generate a [runtime](./runtime.md) capable of protecting the input module(s). **These files are strongly coupled.** A single built `runtime.dll` is only compatible with the protected modules belonging to the metadata that runtime was built with.

Because the Theia packer runs locally on your own machine, your unencrypted binary and debug symbols never leave your machine. The metadata file only contains the necessary information to protect your binary, such as the list of imported symbols, potential stolen functions, and the encryption transformations applied to the input binary. This metadata will be baked directly into the built runtime.

The Theia packer is provided to you by your Zero IT Lab representative and generally does not require frequent updates. Please reach out if you require another copy or have any questions related to the packer.

To get started with the Theia packer, we recommend that you follow the [getting started](../guides/getting-started-cpp.md) guide. This explains how to use the Theia packer and how to submit the results to Zero IT Lab. We also recommend that you review the list of [major features](../features/readme.md) that Theia offers so that you can make an informed decision about which features to enable for your product.

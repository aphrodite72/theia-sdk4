# Custom Crash Dump SDK

The Theia runtime ships with a crash dump creator capable of creating accurate crash dumps for crashes caused by both the Theia runtime itself and your own application or dependencies. If your application requires more than the facilities exposed by the Theia crash dump creator, we also provide an optional C++ SDK that can be used to write a custom dumper interface. Note that we **strongly recommend** that you use the Theia-provided dumper if possible, as it allows for greater security and better crash dumps than can be provided by an external dumper.

It is important to note that by making use of the Theia dumper SDK, you don't actually need to write any dumper logic yourself. The Theia SDK exposes a callback-based system that you can use to interact with the dumper embedded in the Theia runtime. This ensures that the crash dumps contain all information required and that no unnecessary access to the program state is exposed to external programs (which in turn could be compromised or abused by an attacker).

The Theia dumper SDK is shipped in the SDK repo under the `dumper` subfolder, with an example bare-bones dumper program in `dumper/example`. We currently only provide an implementation for C++. To use a custom dumper, build it using the SDK, ship it as an executable alongside your protected application, and ensure that the `dumper_implementation` is set to `Custom` in your [runtime configuration](./configs/runtime-config.md).

For questions about the dumper SDK, please feel free to contact your Zero IT Lab representative.

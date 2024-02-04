# Theia SDK

The Theia SDK is a set of libraries that you can use to interact with the Theia runtime. These allow you to receive notifications about actions taken by Theia, and to request integrity checks, encryption state, and other secure features provided by the Theia runtime.

**Integration with the Theia SDK is required**, even if you do not interact with it. By integrating the Theia SDK, you ensure that your program has certain features set that the Theia runtime needs in order to properly protect your binary.

## Major Features

The primary features exposed by the Theia SDK are the following:

### Crash Callbacks

Whenever a detection or sanity check in the Theia runtime is triggered, the default action is to crash the current process with a code that identifies the crash reason. To allow you to control this behavior, the Theia SDK exposes a "crash callback" system. Whenever a detection is triggered, your protected application can be invoked with information about the crash. This allows you to report the crashes to your server, and even to stop the crash from killing the process entirely.

For more information about SDK callbacks, please see their [dedicated page](./callbacks.md).

### Telemetry Callbacks

Adjacent to crash callbacks, the Theia runtime also exposes "telemetry callbacks". These are callbacks that inform your process about certain actions performed by the runtime, but whose actions you cannot influence. For example, the Theia runtime may emit a telemetry callback whenever it blocks an unsigned DLL from attempting to load in the process.

For more information about SDK callbacks, please see their [dedicated page](./callbacks.md).

### SDK Interface

The Theia SDK exposes an "interface" that can be used to talk to the Theia runtime from your application. This allows you to query whether you're currently running a protected binary, and to ask the Theia runtime to perform integrity checks or other security-sensitive features.

The actions exposed by the SDK interface depend on the version of the SDK. Review the set of available methods for the SDK that you're using for an up-to-date version.

### Protected heap

Theia SDK provides specialized heap implementation that allows for protection from external memory reads and writes of sensitive data.

For more information about protected heap, please see [dedicated page](./heap.md).

## SDK Implementations

We currently provide an SDK implementation for the following languages:

- [C++](./cpp.md)

If you'd like to integrate Theia into an application that does not allow interfacing with any of the above-listed items, please contact your Zero IT Lab representative to discuss possibilities.

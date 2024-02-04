# SDK Callbacks

The Theia SDK distinguishes the following types of callbacks dispatched to your application:

- **Crash callbacks** indicate that the Theia runtime wants to trigger a crash. The protected application can return a different status code to prevent the crash.
- **Crash notify callbacks** indicate that the Theia runtime is going to trigger a crash. The protected application is unable to influence whether the crash actually happens.
- **Telemetry callbacks** communicate to the protected process that the Theia runtime has performed a certain non-fatal action.

We recommend that you use the ability to receive callbacks solely for telemetry reasons. That is, we recommend that you do not prevent crashes by using crash callbacks. In such cases, you're better off [configuring the runtime](../configs/runtime-config.md) to never perform such detections in the first place. This will ensure that the Theia runtime does not spend CPU cycles on detections that will never be used.

## Receiving Callbacks

The Theia SDK will only emit callbacks if you statically register them through the Theia SDK. The exact method for this depends on which programming language you use.

In broad lines, you must register the callback with the Theia runtime. This will ensure that the Theia runtime compiles the callback dispatching logic for the appropriate events. The Theia runtime will only contain code to dispatch the exact set of callbacks you have registered. After registering, you must indicate to the Theia runtime that you are ready to receive callbacks. This can be done by invoking the appropriate method on the Theia SDK interface exposed by the Theia SDK. This ensures that the Theia runtime will not invoke your callbacks before you are ready to receive them.

For specific instructions on how to register callbacks in your language, please refer to the Theia SDK documentation for your programming language.

## Identifying Callbacks

Each callback is parameterized by four values:

- `Code`: A [custom NTSTATUS](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/defining-new-ntstatus-values) uniquely identifying the cause of this callback.
- `Param1`: A 64-bit field whose meaning depends on `Code`.
- `Param2`: A 64-bit field whose meaning depends on `Code`.
- `Param3`: A 64-bit field whose meaning depends on `Code`.

The Theia SDK for your programming language exposes a list of valid NTSTATUS values corresponding to crash (notify) callbacks and telemetry callbacks, as well as documentation on the specific meanings of each of the parameter fields per callback.

Each Theia callback NTSTATUS is built from two components:

- A "category", identifying the general category of the crash or telemetry callback.
- An "index", uniquely identifying the event within the category.

For example, the crash callback emitted when the Theia runtime detects a virtual machine through the CPUID instruction may be exposed as `theia::CrashCode::AntiVM::CPUIDBrand`. Here, the category of the crash is `AntiVM` and the specific index is a numerical value corresponding to the `CPUIDBrand` detection.

## Callback Order

Callbacks can be registered for a specific event, a category of events, or for all events. They are invoked from most-specific to least-specific. For example, a crash with the status `AntiVM::VMDriver` will first attempt to invoke the handler for `AntiVM::VMDriver`, followed by the handler for `AntiVM`, followed by the catch-all handler `AllCrashes`.

Crash callbacks are _short-circuiting_. Returning anything other than the `Default` action from them will result in remaining handlers being skipped. Crash notify callbacks and telemetry callbacks do not have return values and will always invoke all relevant callbacks for their status code.

If you have more than one protected module (e.g. both `game.exe` and `libgame.dll`), each protected module is allowed to register callbacks for the same status. In this case, Theia will invoke the handlers **in the order in which the modules were packed**. The same short-circuiting behavior applies for crash handlers in different modules.

For example, consider the following situation:

- `game.exe` has handlers for `AntiVM`.
- `libgame.dll` has handlers for `AntiVM::VMDriver`, `AntiVM`, `AllCrashes`.
- `game.exe` was packed first, followed by `libgame.dll`.

In this case, crash handlers will be invoked in the following order. If any handler returns a value other than `Default`, the remaining handlers will not be invoked.

- `libgame.dll`'s handler for `AntiVM::VMDriver` (most specific).
- `game.exe`'s handler for `AntiVM` (next most specific, packed first).
- `libgame.dll`'s handler for `AntiVM` (same specificity, packed second).
- `libgame.dll`'s handler for `AllCrashes` (catch-all handler).

## Callback Best-Practices

The following describes some of the behaviors of callbacks and the runtime that the reader needs to be aware of.

### Callbacks are synchronous

Crash and telemetry callbacks are invoked synchronously, on whichever thread they happen to trigger on. Performing long blocking actions in your callback may result in adverse effects on the state of the process, such as the blocking of your render loop, holding a lock for an excessive amount of time, or causing subsequent anti-tamper tasks scheduled for the same thread to time out. It is recommended that you queue any long or expensive tasks to run on a different thread, so that you can return from the handler in a timely manner.

### Not all crashes invoke callbacks

In some cases, the Theia runtime may detect an integrity violation that makes future execution impossible. In these cases, the Theia runtime will not invoke crash callbacks but immediately perform a crash instead. In other cases, it may invoke these crash callbacks but ignore their result. The specific cases in which these can happen are documented within the SDK documentation of your language.

### Prefer crash notify callbacks over crash callbacks

If possible, prefer using the notify variant of crash callbacks (i.e. the version that does not allow you to influence the result of the crash). This will make your application more secure, as an attacker cannot hijack your crash callback and use it to ignore crashes. It will also result in a smaller runtime binary, as notify callbacks result in more optimal code generation.

### Callbacks can influence the size of the runtime binary

It is tempting to register callbacks for entire categories, or even for all events. However, registering such callbacks will result in the Theia runtime having to generate code for dispatching every potential crash. This can result in a large binary size overhead, possibly even doubling the size of the resulting runtime binary (as a result of the obfuscations we automatically apply to the runtime). If you are concerned with the size of the Theia runtime, review whether you have callbacks registered for an excessive amount of events.

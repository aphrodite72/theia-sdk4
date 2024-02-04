# Function stealing

As an optional feature in the Theia packer, you can enable _function stealing_ on your binary to increase protection against dumping attacks. Enabling function stealing will allow the packer to randomly select applicable functions from your input module, and extract ("steal") them. Within your binary these functions are replaced by a simple stub that will call the Theia runtime to execute the appropriate function. The real bodies of the stolen functions will instead live in the `runtime.dll` built by the Bifrost platform.

**Why function stealing?**  
The function stealing feature exists to make it harder for attackers to create a standalone version of your application with Theia removed. Function stealing adds an additional layer of complexity to this process, because not all functions in your binary can be found in their original representation. Function stealing may additionally help against attackers that rely on byte signatures for offset detection, as well as provide an additional barrier for static analysis of a memory dump of your protected process.

**Performance costs of function stealing**  
Stolen functions will be placed inside the `runtime.dll` as-is. This means that a call to a stolen function only occurs one additional indirect branch. Under modern CPUs, even these indirect branches will be appropriately branch predicted. As a result, enabling function stealing is effectively free in terms of performance costs.

**Does Theia perform any modifications/processing on stolen functions?**  
Stolen functions are inserted in the produced `runtime.dll` as-is. They will not be obfuscated or otherwise incur extra runtime overhead. You do not have to worry about potential performance bottlenecks, even if a hot function is stolen.

**What functions can be stolen?**  
A function is considered as eligible for stealing when all of the following conditions apply:

- The function must be defined in the input PDB
- The function range must consist entirely of statically traceable valid instructions (e.g. no padding bytes, indirect jumps within the function)
- The function must not perform (un)conditional jumps to instructions outside of the defined function range
- The function must not have any instructions that refer to IP-relative addresses (e.g. direct function calls, globals)
- The function must not have a complex unwind frame (e.g. the function must not use `__try/__catch/__finally`)
- The function must unconditionally write to a caller-save register

Effectively, this generalizes to functions that do not refer to globals and do not call other functions directly. Indirect calls (e.g. virtual calls) and control-flow within the function is allowed and does not disqualify a function from being stolen.

Stolen functions in the original binary are replaced by a "trampoline" that transfers control-flow to the stolen function inside the Theia runtime. This trampoline requires a single hardware register to store an intermediate value. The packer will select this register from the registers that the stolen function writes to, ensuring that there is no observable change or violation of the calling convention (whichever that convention may be). If no such register can be determined, the function will be marked as ineligible for stealing.

**How do I control which functions are (not) stolen?**  
The Theia module configuration used by the packer allows you to configure which functions should always and/or never be stolen, as well as the maximum amount of functions stolen. For more information, review the [module configuration documentation](../configs/module-config.md).

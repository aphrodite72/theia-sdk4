# Major Features

This section documents some of the major features available in the Theia runtime. These features, when enabled, provide additional protection for your binary. However, not all of them are suitable for every type of binary. Within this section, each of these major features is elaborated upon so that you may make an informed choice.

We provide extra elaboration for the following major features:

- [Full-read tracking and mid-instruction decryption rejecting](./full-read-tracking.md)
  - Stops attackers from decrypting your application and makes it resistant against all publicly known executable dumpers.
- [Nanomites](./nanomites.md)
  - Removes critical information from decrypted pages at the cost of a small runtime performance hit, making it significantly harder for an attacker to reverse your application once dumped.
- [Function stealing](./function-stealing.md)
  - Makes it significantly harder for an attacker to remove Theia from your binary, by stealing arbitrary functions from your input module and placing them in the Theia runtime instead.
- [Process integrity options](./process-integrity.md)
  - A set of options which can defend against rogue code in your process, but are potentially harmful to legitimate injecting software such as overlays and gameplay capturing software.

Each feature within this section can be configured in the Theia [module](../configs/module-config.md) and [runtime](../configs/runtime-config.md) configuration.

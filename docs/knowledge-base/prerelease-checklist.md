# Pre-release checklist

Before releasing your application with Theia, you should ensure that your configurations and files are appropriate and that you do not include files that are helpful to cheaters. This document contains a checklist of things that we recommend you check before a release.

## Release archive files

Certain Theia files are only needed for building/packing, and should not be shipped to users. Confirm that your total release archive adheres to the following:

- [ ] No intermediate artifacts (`metadata.bin`, `customer.log`) are present in the release archive.
- [ ] No tooling related files (`packer.exe`, `agent.exe`, `diagnoser.exe`) are present in the release archive.
- [ ] No tooling related config files (`module-config.json`, `runtime-config.json`, `agent.toml`) are present in the release archive.
- [ ] No tooling related authentication files (`client.p12`, `client.pem`) are present in the release archive.
- [ ] No original (unprotected) executables (`exe`, `dll`) are present in the release archive.

Outside of Theia-related files, compilation artifacts can also help attackers to reverse-engineer your software. We strongly recommend that you check that:

- [ ] No Program Debug Database (PDB) files are present in the release archive.
- [ ] No linker map (MAP) files are present in the release archive.
- [ ] No object files (OBJ) or libraries (LIB) are present in the release archive.

## Theia integration specifics

Theia has a large number of settings, some of which have user-facing impact. The following are some good guidelines on a proper Theia implementation:

- We strongly recommend to opaquely crash (i.e., without a message to the user) on detections that can only be caused by targeted direct attacks, such as anti-debugger checks. The user does not need to be informed that they should not be debugging the game, this is common sense.
- Do not have software code or backend systems that reveal unnecessary amounts of Theia functionality to the public. For example, you should prefer submitting opaque Theia crash codes to the server, and only resolving their meaning server-side. This prevents an attacker from learning about the inner workings of Theia, and how they might bypass its detections.
- Do not release Theia builds with debug settings. Debug builds should only be used to test Theia integration. To discourage this, a special splash screen is added if any debug option is enabled.

## Unreal Engine specifics

If you are using Unreal Engine, we additionally recommend the following:

- We highly advise you integrate our Unreal Engine hardening patches. Not doing so means that you will be leaking reflection metadata and type information, which are extremely useful to attackers.
  - If using the hardening patches, ensure that you are using them for all platforms (including consoles and dedicated servers). The license of the patches allows you to do this free of charge as long as you have an active Theia subscription, and builds that only integrate the patches do not count against the MAU. Not releasing any binaries without the patches is crucial, as otherwise attackers will be able to correlate the "clean" metadata with the obfuscated metadata.
- If you are not using the UE hardening patches, avoid putting Theia-related information (e.g. detection reasons, crash codes) in objects that use UE's reflection system (e.g. UObject, UEnum). This avoids exposing information about Theia and its functionality.

## User experience and support

While not strictly necessary, we advise you set up user facing messaging and support channels regarding security systems. Having well written user facing messaging is especially important if you are enforcing settings that may have non-zero benign occurrences in the wild, like secure boot enforcement or vulnerable driver detection. Unless otherwise configured using the runtime-config or SDK callbacks, Theia will default to an intentional crash if security violations are detected. In order to help users if they encounter issues, consider the following:

- If you are using the default Theia crash dumper, ensure that you configure `support_instructions` to contain actionable steps for the user, such as a link to your support portal.
- If you are using the default Theia crash dumper and a crash monitoring platform (e.g. Sentry, Bugsnag, etc.), configure HTTP crash uploads in the Theia crash handler to automatically receive crash dumps.

Note that revealing information about Theia is a balancing act, as any public information about detections will help attackers avoid them. As a general guideline, we recommend only revealing information related to detections that an average user may encounter unintentionally (e.g. secure boot is off, a vulnerable driver was detected, certain badware was detected). We strongly recommend that you do not disclose any information about security-related violations, such as integrity violations or malicious hypervisors. Users will only encounter these if they are actively attempting to break protections, and giving them information on what detection they tripped will only help them avoid it in the future.

## Set realistic expectations

If you've implemented the Theia SDK in your application and carefully reviewed the points above, you are close to releasing your newly protected application into the wild! While Theia will be hard at work protecting your application from attacks and attempts at reverse engineering, we strongly recommend that you review our documentation on [what to expect](./expectations.md). Theia helps make your application more secure, but it is not a magic bullet. An optimal protection will depend on proper action, both from our side and yours.

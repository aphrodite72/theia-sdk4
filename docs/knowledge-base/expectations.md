# Expectations

> **Note**: Although this document is written from the perspective of protecting a video game, it contains general information relevant to all applications protected by Theia. We strongly recommend that you review this document even if you are not using Theia to protect a video game.

Ever since the dawn of video games, cheating has been a constant threat faced by developers. The battle against cheaters has tremendously evolved over the last decades, and now has an immense amount of complexity. This document hopes out outline some of the challenges you'll be facing when securing your game, and detailing in which areas Theia can and can't help.

## Cheat types

Generally, we identify three distinct groups of cheats:

- Internal: the cheat is directly mapped inside the game process and can freely read and write memory.
- External: the cheat is running outside the game process, reading (and sometimes writing) game memory remotely. External cheats come in various flavors. An incomplete list:
  - A user-mode process that uses Windows APIs like [ReadProcessMemory](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-readprocessmemory) and [WriteProcessMemory](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-writeprocessmemory) to interact with the game process.
  - Vulnerable drivers: a cheat may load and exploit a known vulnerable kernel driver to map a malicious kernel driver.
  - Direct Memory Access (DMA): specialized hardware devices may be used to achieve access to game memory.
  - Custom hypervisors: a hypervisor that implements functionality designed to aid cheating.
  - (UEFI) Bootkits: control flow is hijacked early in the boot process to inject malicious functionality at hypervisor or kernel-level.
- Presentation/input layer: game memory is not directly accessed. Instead, the cheat reads the screen to gather information. Occasionally, external devices (e.g. Arduino) may be used to inject inputs to the game.
  - Pixel/color bots: simple software systems designed to identify player outlines based on their highlight color.
  - AI aim/trigger bots: machine vision systems (neural network) designed to identify opposing player characters.
  - Macros/anti-recoil: an external device may be programmed to counter-act weapon recoil or perform multiple inputs at once.

## Trends

While every cheat is different, the capabilities and effectiveness of each cheat can be roughly derived from its type. In particular:

- Internal cheats tend to be the most functional (and fastest) but also the easiest to detect.
- External cheats tend to be less functional than internal (and slower) but are still fairly capable and harder to detect.
- Presentation layers cheats tend to be the least functional (and slowest) but they can be almost impossible to detect.

From our perspective, if you can prevent the vast majority of internal and external cheats for a game, you are actively winning the battle against cheating.

## Cheat development

A significant chunk of games utilize one of two specific game engines: Unreal Engine or Unity. Unfortunately, both game engines contain type information and memory layout information at runtime. While this helps ease game development, it is also a significant source of information for cheat developers. Commonly, cheat developers extract type and layout information from the game (requiring very little reverse engineering skill) and use this to create a "Software Development Kit" (SDK) for the game.

Through the SDK, access to the game and its entities and objects is entirely abstracted away. This means that a cheat developer only needs to write the actual cheat logic (which is commonly game agnostic, e.g. aimbots or ESPs), and some way to interact with the game. This final ingredient is either:

- Some way to inject arbitrary code into the game process (internal), or
- A method that allows arbitrary read/write access to game memory (external).

Big cheat providers often already have these ingredients around and can use the SDK to quickly and easily bootstrap functional cheats for newly released games, often finishing a first prototype within the first day of public release.

## Big picture strategies

From our perspective, the fight against cheating from a game developer's end involves three different aspects:

- Prevention: software measures designed to slow down cheat development.
- Detection: detect cheating when it happens.
- Deterrence: sue or otherwise shut down cheat developers.

While the priority between these varies, we strongly recommend investing resources into all three if possible. If not, putting efforts into the prevention of cheat development is often the most effective as this directly leads to less cheats that need to be detected.

## Vendor services

There are several vendors available that can help safeguard the competitive integrity of your game. These vendors typically offer services geared towards both preventing and detecting cheats. While detection services can identify cheats once they are active, they don't necessarily stop cheats from being created initially. Cheat developers can often easily circumvent detection services, which might prevent them from playing online but still allows them to inject code and use debuggers during development. Detection services become more beneficial once cheats are already being circulated and sold.

### Theia

As part of every Theia subscription, we provide access to our Unreal Engine hardening patch, which removes the vast majority of type information from the game executable. Generating an "SDK" from these hardened game executables is possible, but the output is significantly less useful: type and name information is irrecoverably lost.

Assuming engine level hardening is in place, data-only attacks (such as SDK generation) are much less effective and the attacker has to resort to actually looking at the executable, which is where Theia comes in. Theia provides a very reasonable level of protection for its targeted performance overhead (~1%), obfuscates executable code in memory, and makes reverse engineering more time-consuming and annoying.
Nevertheless, it is still just obfuscation. You should always assume that a (dedicated) attacker will eventually be able to retrieve and reverse engineer game code.

So... what's next in terms of "prevention"? Some of our partners are deploying Custom Executable Generation (CEG) technology, meaning they build multiple copies of their games. These hardened builds have various aspects of the executable randomized (including memory layout, vtable ordering, encryption routines) as well as Theia protection applied. These randomized executables are then distributed to players. When properly executed, the result is that developing, maintaining, and distributing both internal and external cheats is significantly more challenging, to the point where its no longer practical at scale.

## Conclusion

Having a detection service is already standard in the industry. However, detection services can be overwhelmed when numerous cheat providers operate on a game, so having effective preventative measures (like Theia) is beneficial. If your game genre is one where cheating is highly sought after (e.g. FPS), it is most beneficial to have _both_ Theia and CEG technology.

Solely using Theia will not stop all internal and external cheats from being developed. Engine level hardening (especially with additional first-party effort) and CEG go a long way to multiply the usefulness of Theia. While Theia is constrained by supporting nearly every use case, first party measures can focus on a narrow subset of supported configurations, and may employ compiler or build environment specific means. We are happy to give advice or feedback on any such steps.

We hope this overview was decent. We encourage you to start a conversation with us if you have any questions or need advice on what might be the best fit for your game.

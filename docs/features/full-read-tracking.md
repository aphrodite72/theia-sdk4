# Full read tracking and mid-instruction decryption rejection

This document outlines the "full read tracking" protection that Theia offers. It aims to serve as a simple introduction to the way in which Theia protects your binary, followed by an explanation of what exactly full read tracking is and how using it can significantly improve the security of your binary. If you have any questions about full read tracking or are having trouble implementing it in your binary, please feel free to contact us.

The end of this document also briefly discusses [mid-instruction decryption rejection](#mid-instruction-decryption-rejection), an additional hardening technique that is adjacent to full read tracking and usually enabled alongside it.

## An introduction to full read tracking

### How Theia protects your binary

A module protected by Theia will have the majority of its content encrypted, even at runtime. Theia accomplishes this by artificially marking the encrypted sections of the binary as "no access". Whenever the processor attempts to read from or execute such a protected "page", the operating system will raise an error. Theia intercepts this error and decrypts the page, allowing the execution to continue as if the page was never encrypted. After some time has passed, Theia will automatically re-encrypt the page. This will ensure that your binary never exists in memory fully unencrypted, making it impossible for an attacker to obtain a fully decrypted version of your executable.

Theia encrypts your binary in increments of 4 KiB. Each such section is colloquially called a page. The packer will mark each page in your executable as either plaintext, load-encryption, or continuous encryption, based on the contents of the page.

Plaintext pages will not be encrypted by the packer. These are pages that must be readable to the operating system before Theia has a chance to decrypt them. Examples of such pages include the binary metadata and resources such as application icons.

Load-encryption pages will be encrypted by the packer, but immediately decrypted by the packer upon startup. After they have been decrypted, their contents will never be re-encrypted. Examples of content that is read-encrypted are both read-only and read-write content (e.g. string literals, initial values of global variables, etc).

Continuous encrypted pages work as described in the first paragraph of this section. They exist in memory in encrypted form, and will only be decrypted if the processor attempts to access them. Such decrypted pages will be automatically re-encrypted after enough time has passed. By default, all executable content in your binary (e.g. all code) will be continuously encrypted.

### How attackers attempt to dump your binary

The main goal of anyone attempting to attack a packer product such as Theia is not necessarily to reverse-engineer how the packer works (this is largely unfeasible, since each packer build is unique and heavily obfuscated/virtualized). Instead, most attackers will simply attempt to trick the packer into decrypting individual pages until the attacker has a plaintext copy of all pages.

The normal approach for this is for an attacker to trigger "read faults" on encrypted pages. Essentially, the attacker will attempt to read a byte in an encrypted page. Since these pages are marked as "no access", an error (a "fault") will be raised by the processor, which Theia will receive and will use to decrypt the page. Afterwards, the attacker can simply read the entire contents of the page as Theia has helpfully decrypted it for them. Repeat this for all encrypted pages, and an attacker has completed their goal.

### How packers traditionally defend against this attack

Defending against the outlined attack is tricky, as there are legitimate reasons for both your program and the operating system to read encrypted pages. This means that preventing the attack becomes largely a heuristic approach by attempting to distinguish between normal read behavior and an attacker attempting to dump memory. Generally, packers (including Theia), employ the following heuristics to attempt to detect dumping attacks:

- Decryption attempts should only come from a legitimate location
- Decryption attempts are rarely on the first byte of the page
- Decryption attempts are rarely in sequential order
- Decryption attempts are usually spread out during execution, and not in large bursts
- A single location rarely decrypts more than one other page

Such heuristics are often effective at stopping trivial attacks, but it is almost always possible for an attacker to find a mode of attack that can avoid triggering these heuristics. For such a skilled attacker, it is only a matter of time before they are able to dump all encrypted pages.

### How full read tracking protects against this attack

With Theia, we developed an industry-leading protection method that avoids most of the pitfalls described in the previous sections. We call this protection method full read tracking.

Full read tracking makes use of the fact that the processor indicates whether a protection fault was triggered because of an attempt at _reading_ or an attempt at _executing_ an encrypted page. This is combined with the fact that almost all compilers will separate your program's data from your program's code. A consequence of this separation is that a legitimate program will have very little reason to _read_ the bytes that represent your program's code. Indeed, almost all decryptions should be caused by an attempt at _executing_ the encrypted page.

Theia's packer uses this observation to mark certain pages as "never read". Whenever it can prove that a single page contains only executable code, it will instruct the runtime to deny any attempts at reading the page, allowing only attempts at executing it. If an attacker attempts to dump the page by causing a read fault, the Theia runtime will refuse a decryption and crash the application.

This protection is surprisingly effective even on binaries that are not optimized for it. For example, a standard build of the Unreal Engine ShooterGame example has more than 40% of the executable code protected with full read tracking. An attacker will be **incapable of dumping these pages**, as it becomes impossible to trigger a decrypt by reading them.

Even better, this ratio can be greatly improved by making a few simple changes to your compilation process. The remainder of this document describes how you can increase the amount of pages in your executable protected using full read tracking.

## Optimizing full read tracking for your binary

Getting started with full read tracking in your binary is easy. In fact, you don't even need to do anything at all. Theia's packer will automatically figure out which pages are safe to apply full read protection to, without any further effort on your end.

However, the packer is designed to work on any input binary. This means that it must be conservative in assigning protections. Additionally, most compilers by default will produce "jump tables" for switch statements during optimization, which are data structures located in code sections that must remain readable for the executable to function.

The following sections describe some ways in which you increase and debug full read tracking in your binary.

### Disabling or moving jump tables

The main thing that prevents full read tracking is the presence of _jump tables_ in your binary. These are lookup tables generated by the compiler in order to make switch statements and complex if-expressions faster. By default, most production compilers put these jump tables in the executable section near the function that uses them, so that the table is more likely to be in the processor cache. However, since jump tables are read by code (and not executed), any page that contains them cannot be optimized.

Most compilers offer flags to either move the jump tables to read-only memory, or to disable them entirely, both of which will significantly increase the number of fully read-protected pages.

Note that, for best results, you must apply these flags to all your code, including any possible libraries that you statically link in your final module. That includes your game engine, such as Unreal.

**MSVC**:  
To move jump tables to a read-only section in MSVC, apply the following flags:

- Compiler flags: `/d2jumptablerdata`
- Linker flags: `/d2:-jumptablerdata`

**clang/LLVM**:

- Compiler flags: `-fno-jump-tables`

**clang-cc**:

- Compiler flags: `-Xclang -fno-jump-tables`

### Ensuring your .pdb files are complete

The packer determines which pages are safe to perform full-read protection on by tracing which bytes of the executable sections are part of functions. In order to make this tracing as complete as possible, the packer needs accurate source ranges for all the functions in your binary. These are normally stored as part of the .pdb generated by your compiler.

If your binary is statically linked, your PDB may not contain the complete function ranges from any libraries that you link into the final executable. This will affect the output, as it becomes harder for the packer to detect proper ranges.

Unfortunately, there is no single set of instructions on how to achieve this. It will depend on your build process and libraries used. Please feel free to contact us if you need pointers on how to improve your build PDBs.

### Ensuring that you do not have data in your r-x sections

Global variables and constants are generally put in read-only and read-write sections. However, it is possible to override this on a per-variable basis. Review your code-base and check whether any of your variables are placed in the `.text` section, or any other r-x section. Consider moving them to a different section, if appropriate.

### Review use of binary packers or obfuscation tools

Theia generally does not work correctly with the code generated by packers. This includes anti-tamper tools, as well as compressing packers such as upx. It is very likely that any such tools may result in a broken binary.

Theia is explicitly compatible with binaries that have been virtualized using Griffin 0.5 and newer. Full Theia support was added in a minor patch, so you may need to rerun Griffin. An attempt to use a binary processed by an older version of Griffin will result in an error message that looks like this:

```plain
The binary contains an invalid unwind info address: 0xAABBCCDD. Address does not exist or lies in executable memory.
```

Please contact us if you are having trouble protecting a binary processed by Griffin.

## Debugging full read tracking

If your program crashes randomly and no crash handler is invoked, it may be caused by the packer refusing to decrypt. Such crashes will usually manifest as a `STATUS_ACCESS_VIOLATION` (0xC0000005), although that is not always the case. If it manifests as an access violation, you can confirm whether or not it was caused by a refused decrypt by checking whether the target address resides in a no access memory page.

Crashes caused by full read tracking are always caused because some code attempted to read a byte in an r-x page. This is generally not something that happens accidentally, which means that there are avenues of detecting potential code paths that can trigger a full read protection fault.

Review both your code, and any dependencies, to see if any of them contain or perform any of the following:

- Function integrity checks or function hook detections.
- The hooking of functions within the protected module.
- Unwinding or producing a backtrace without using the appropriate Windows APIs.
- Calling `VirtualProtect` on R-X sections.
- Hand-written assembly that references constants in R-X instead of R-- memory.

If you notice that a library dependency is causing any of these issues, and you are not able to (easily) rectify them in the library yourself, please feel free to reach out to us and we can potentially look at making the Theia packer aware of this library and whitelisting it accordingly.

## Mid-instruction decryption rejection

As a side-effect of our advanced binary tracing employed by the packer, we're able to reconstruct exactly which bytes in your binary represent the _start of an instruction_. Instructions in your binary are almost always multiple bytes long, but a normal binary will only perform decryptions originating on the start of of an instruction†.

Theia can use this property to further reduce the possible methods for attackers to decrypt your binary. If enabled, Theia will ensure that all decryptions originate from the start of an instruction, and that they only attempt to decrypt the start of another instruction. This makes it harder for an attacker to use "gadgets" in existing decrypted pages to decrypt new pages.

Mid-instruction decryption rejection has no observable performance penalty when enabled. However, Theia runtime builds with the feature enabled may have a larger binary size due to the additional binary metadata compiled into the runtime. This metadata is encrypted and validated on startup, and an attacker will not be able to use the information for anything useful even if they manage to decrypt the information.

Mid-instruction decryption rejection may not be fully compatible with hand-written assembly or obfuscated code. In particular, code that jumps into the middle of an instruction in a way that is not statically traceable may result in a false positive. Such cases are exceedingly rare, and we have not observed any incompatibilities with common libraries and game engines. If you encounter issues that disappear when you disable mid-instruction decryption rejection, please reach out.

> †: With one notable exception: when an instruction crosses a page (4kb) boundary. In those cases half of the instruction might already be decrypted. Theia specifically allows mid-instruction decryptions when this is the case.

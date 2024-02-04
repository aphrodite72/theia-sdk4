# Diagnoser

Whenever the Theia runtime detects an integrity failure, encounters an internal error, or detects any of the configured detections, it will cause a process crash (unless [crash callbacks](../sdk-documentation/callbacks.md) are configured). To prevent attackers from learning exactly what caused a Theia-induced crash, Theia intentionally deletes the recent context and encrypts the crash parameters.

As part of the Theia SDK, we offer the `diagnoser` command-line tool for diagnosing such crashes. Given a crash dump or register state, the diagnoser tool will indicate the reason for the crash and optional additional information. The diagnoser tool will be provided to you by your Zero IT Lab representative.

An example output of the diagnoser tool is as follows:

```shell
$ diagnoser dump C:/Users/user/AppData/Local/Temp/PACKER-00000.dmp
 INFO diagnoser: This dump is of a process with a Theia instance loaded:
 INFO diagnoser: - Bifrost job ID: ffeeddccbbaa99887766554433221100
 INFO diagnoser: - Process uptime: 10s
 INFO diagnoser:
 INFO diagnoser: Detected a Theia soft crash at 0x7ffe9d200000.
 INFO diagnoser: The process crashed with status 0xe0670103.
 INFO diagnoser: This represents an assertion failure within the Theia runtime.
 INFO diagnoser:
 INFO diagnoser: Assertion failed at 43e28b05dd7d4d00:429.
 INFO diagnoser: Assertion tag: 61626364 (abcd)
 INFO diagnoser:
 INFO diagnoser: The Theia runtime encountered an unexpected NTSTATUS code: c0000022
 INFO diagnoser: STATUS_ACCESS_DENIED: {Access Denied} A process has requested access to an object, but has not been granted those access rights.
 INFO diagnoser:
 INFO diagnoser: If you believe this to be a Theia bug and not a result of faulty, misconfigured, or resource-starved hardware, please contact Zero IT with your dump file.
```

## Usage

The diagnoser tool is a command-line application meant to be used from a command-line terminal. The following usages are supported:

- Diagnosing a crash using a minidump file created by the Theia dumper or another crash reporter tool:

  ```shell
  $ diagnoser dump <path to .dmp file>
  ```

- Diagnosing a soft crash (exception address ends in `0000`) by manually providing values for the 4 crash parameter registers:

  ```shell
  $ diagnoser soft <rdx> <r8> <r9> <r10>
  ```

- Diagnosing a hard crash (exception address ends in `0001`) by manually providing values for the 4 crash parameter registers:

  ```shell
  $ diagnoser hard <rdx> <r8> <r9> <r10>
  ```

For a full overview of all available options, use the `--help` flag: `diagnoser --help`.

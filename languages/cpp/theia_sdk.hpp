/// @file theia_sdk.hpp
/// @copyright Copyright (C) Zero IT Lab - All Rights Reserved
/// @brief SDK for Theia.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential
#pragma once

#include <cstdint>
#include <cstdlib>
#include <type_traits>

typedef long NTSTATUS;

#if defined __has_include
#if __has_include(<vcruntime.h>)
#include <intrin.h>
#define THEIA_NOP() __nop()
#endif
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#define THEIA_EXPORT     __declspec(dllexport)
#define THEIA_SECTION(x) __declspec(allocate(x))
#else
#define THEIA_EXPORT
#define THEIA_SECTION(x) [[gnu::section(x)]]
#endif

#ifndef __clang__
#pragma section(".data")
#pragma section(".rdata")
#pragma section(".text")
#endif

#if !defined(THEIA_PREFIX)
#define THEIA_PREFIX DEFAULT
#endif

#define THEIA_CONCAT_(a, b)  a##b
#define THEIA_CONCAT(a, b)   THEIA_CONCAT_(a, b)
#define THEIA_REAL_NAMESPACE THEIA_CONCAT(theia_, THEIA_PREFIX)

namespace THEIA_REAL_NAMESPACE {
  /// The Theia SDK is a stable interface to a constantly changing Theia runtime. Within a
  /// single major release, Zero IT Lab provides the guarantee that each past release of the
  /// Theia SDK will remain backwards compatible with any version of the Theia runtime.
  ///
  /// To be able to realize this guarantee, some of the enumerations inside this file are
  /// documented to be "non-exhaustive". For these, your code must be able to handle values
  /// outside of the documented range. This allows us to add new detections, badware, and
  /// crashes to the Theia runtime even if your application does not yet know about them.
  ///
  /// Occasionally, the Theia runtime will expose new functions to your application through
  /// the interface provided by GetInterface(), or change the behavior of existing functions.
  /// When this happens, the THEIA_INTERFACE_VERSION will be incremented. The Theia runtime
  /// will use this version to ensure that the behavior of any specific version of the
  /// interface stays consistent, even if new features are exposed only for newer versions.
  ///
  /// We strongly recommend that you update the SDK whenever a new version is released.
  /// New SDK versions introduce new constants, crashes, or other code which your application
  /// may want to handle differently. In most cases, an SDK update requires no other code
  /// changes on your end.
  enum : uint32_t {
    /// @brief The major version of this Theia SDK.
    THEIA_MAJOR_VERSION = 2,

    /// @brief The interface version for communication between the SDK and the runtime.
    THEIA_INTERFACE_VERSION = 1,
  };
}
namespace theia = THEIA_REAL_NAMESPACE;

// This namespace declaration contains implementation details and is not relevant to the use of the
// Theia SDK. It is recommended that you collapse this declaration, if your editor supports it.
namespace THEIA_REAL_NAMESPACE {
  struct FunctionPtrs;

  namespace detail {
    template <uint32_t MajorVersion, uint32_t InterfaceVersion>
    struct TheiaSDKIdentifier {
      THEIA_EXPORT void DoNotCall();
    };

    THEIA_EXPORT extern const FunctionPtrs* g_TheiaVMT;

    THEIA_EXPORT extern const uint8_t g_DataBlob[0x2000];
    THEIA_EXPORT extern const uint8_t g_RdataBlob[0x2000];
    THEIA_EXPORT extern const uint8_t g_TextBlob[0x2000];

    class ForceDynamicInitializer {
      void* volatile p;

    public:
      ForceDynamicInitializer() {
        p = nullptr;
      }
    };

    extern thread_local ForceDynamicInitializer g_ForceDynamicInitializer;

    template <uintptr_t UniqueId>
    struct PageGuardian {
      THEIA_EXPORT void DoNotCall();
    };

    // enum flags describing a crash callback
    enum : uint32_t {
      // this is a telemetry callback instead of a crash callback
      THEIA_CALLBACK_FLAGS_TELEMETRY = 0x1,

      // this is a callback for a category
      THEIA_CALLBACK_FLAGS_CATEGORY = 0x2,

      // this callback returns void
      THEIA_CALLBACK_FLAGS_NO_RETURN_VALUE = 0x4
    };

    template <uint32_t Number, uint32_t Flags, typename ReturnType>
    struct CrashCallback {
      THEIA_EXPORT static ReturnType Call(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3);
    };

    template <size_t N>
    constexpr uint64_t Fnv64a(const char (&String)[N]) {
      uint64_t Hash = 0xcbf29ce484222325;
      for (size_t i = 0; i < N; ++i)
        Hash = (Hash ^ static_cast<uint8_t>(String[i])) * 0x100000001b3;
      return Hash;
    }

    enum class AllCrashSentinel { SubscribeToAllCrashes };
    enum class AllTelemetrySentinel { SubscribeToAllTelemetryNotifications };
  } // namespace detail
} // namespace THEIA_REAL_NAMESPACE

/// Main implementation of the Theia SDK. It is recommended that you fully review the documentation of
/// all code items within this namespace, as they provide important information on the use of the Theia
/// SDK. To initially get started with the SDK, you are recommended to review the THEIA_ONCE and
/// THEIA_REGISTER_CALLBACK macros.
namespace THEIA_REAL_NAMESPACE {
  /// @brief A general category of crashes. Each theia::CrashCode entry belongs to a single category.
  ///
  /// Each crash caused by Theia will result in an NTSTATUS that consists of both the category of
  /// the crash, as well as a unique identifier within that category.
  ///
  /// You are able to subscribe to crash callbacks for an entire category. When done so, you will
  /// receive crash callbacks for all possible crashes within that category. The NTSTATUS parameter
  /// for those crashes represents the full crash. To extract the category from that status, use
  /// theia::CrashCode::GetCategory.
  ///
  /// @warning This enumeration is not exhaustive. Newer versions of the Theia runtime may report
  ///          values outside this enumeration without a major version update.
  enum class CrashCodeCategory : uint8_t {
    /// @brief Invalid.
    Invalid = 0,

    /// @brief An assertion failed.
    /// @note Assert crashes cannot be stopped, even with a callback, and will always result in a soft crash.
    Assert,

    /// @brief A virtual machine or hypervisor detection was triggered.
    AntiVM,

    /// @brief A debugger or anti-anti-debugger was detected.
    AntiDebug,

    /// @brief An invalid machine configuration, such as an unsafe boot configuration, was detected.
    MachineIntegrity,

    /// @brief The protected binary or Theia runtime failed integrity checks.
    /// @note Code integrity failures will not invoke crash callbacks since the integrity of the
    ///       Theia runtime is considered a prerequisite for the proper functioning of the runtime.
    CodeIntegrity,

    /// @brief The protected process failed integrity checks, such as untrusted modules or external threads.
    ProcessIntegrity,

    /// @brief Periodic tasks executed by the Theia runtime were corrupted or interrupted.
    Periodic,

    /// @brief Multiple instances of the same protected executable were detected.
    SingleInstance,

    /// @brief Badware or other undesired software was detected.
    Badware,

    /// @brief Hardware error was detected.
    HardwareError
  };

  /// @brief The facility for custom NTSTATUS values used by Theia.
  constexpr const uint16_t STATUS_FACILITY_THEIA = 0x67;

  /// @brief A collection of Theia crash codes.
  ///
  /// Theia crash codes are constructed as custom NTSTATUS values that use the `STATUS_FACILITY_THEIA_CRASH`
  /// facility and an error severity. You can use the methods exposed in this namespace to introspect
  /// such statuses. Individual sub-namespaces of this namespace expose individual crash codes.
  ///
  /// The `Code` part of a Theia crash NTSTATUS consists of a 16-bit value. The top 8 bits represent the
  /// crash code category, whereas the bottom 8 bits represent a unique crash within that category. Note
  /// that the unique crash index `0` is a reserved value and will never be reported.
  ///
  /// When a crash callback is invoked, three additional parameters will be supplied which may contain
  /// additional information on the crash. By default, all these parameters are reserved. For some specific
  /// crashes, we explicitly define and expose the contents of the values. For those crashes, the meanings of
  /// the parameters is explicitly documented. If you report crash callbacks to a server, it is recommended
  /// that you always submit all parameters, even for crashes that don't expose any values. Their reserved
  /// values can be used by Zero IT Lab to diagnose crashes.
  ///
  /// @warning The list of crash codes exposed in this namespace is not exhaustive. Newer versions of
  ///          the Theia runtime may report new crash codes not listed in this namespace without a major
  ///          version update. Your code must be able to handle unknown crash codes.
  namespace CrashCode {
    /// @brief Symbolic constant that can be used to subscribe to all crashes.
    ///
    /// @warning It is **strongly** recommended that you avoid subscribing to all crashes,
    ///          especially if you only care about crashes from a few categories. Theia will
    ///          generate significantly more code when a catch-all callback exists, as every
    ///          possible crash site must now invoke the callback. This can lead to much larger
    ///          Theia runtimes.
    constexpr const detail::AllCrashSentinel All{};

    /// @brief Check whether the given NTSTATUS is a custom Theia crash status.
    constexpr bool IsTheiaCrashCode(NTSTATUS status) {
      bool isCustom = ((status >> 29) & 1) == 1;
      bool isErrorSeverity = ((status >> 30) & 3) == 3;
      bool isTheiaFacility = ((status >> 16) & 0xFFF) == STATUS_FACILITY_THEIA;
      return isCustom && isErrorSeverity && isTheiaFacility;
    }

    /// @brief Build a Theia crash status from the given category and index.
    constexpr NTSTATUS MakeCode(CrashCodeCategory category, uint8_t index) {
      auto combined = (uint16_t)((((uint16_t)category) << 8) | (uint16_t)index);
      return (NTSTATUS)((uint32_t)0
                        | (((uint32_t)3) << 30)                               // STATUS_SEVERITY_ERROR
                        | (((uint32_t)1) << 29)                               // CUSTOM
                        | ((((uint32_t)STATUS_FACILITY_THEIA) & 0xFFF) << 16) // FACILITY
                        | ((((uint32_t)combined) & 0xFFFF)));                 // CODE
    }

    /// @brief Retrieve the crash code category from the given status. Return
    ///        CrashCodeCategory::Invalid if not a Theia crash code.
    constexpr CrashCodeCategory GetCrashCategory(NTSTATUS status) {
      if (!IsTheiaCrashCode(status))
        return CrashCodeCategory::Invalid;

      auto value = (uint16_t)(status & 0xFFFF);
      return static_cast<CrashCodeCategory>(value >> 8);
    }

    /// @brief Retrieve the unique crash index for the given Theia crash code.
    ///        Return zero if the input is not a valid Theia crash code.
    constexpr uint8_t GetCrashIndex(NTSTATUS status) {
      if (!IsTheiaCrashCode(status))
        return 0;

      return (uint8_t)(status & 0xFF);
    }

    /// @brief Crash codes caused by an assert or other deliberate crash.
    ///
    /// @warning Crashes within this category cannot be prevented by returning a
    ///          SDKCallbackAction::Continue, as their failure is considered a breach
    ///          of the proper functioning of the runtime. Any value returned by
    ///          callbacks registered for these crashes will be ignored.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace Assert {
      /// @brief A generic Theia runtime assertion failed.
      ///
      /// Param1: Hash of the source filename.
      /// Param2: Line number (bottom 32 bits); optional tag identifying the assert (top 32 bits)
      constexpr const NTSTATUS Failed = MakeCode(CrashCodeCategory::Assert, 1);

      /// @brief A generic Theia runtime assertion failed with an associated value.
      ///
      /// Param1: Hash of the source filename.
      /// Param2: Line number (bottom 32 bits); optional tag identifying the assert (top 32 bits)
      /// Param3: Value associated with the assert.
      constexpr const NTSTATUS FailedValue = MakeCode(CrashCodeCategory::Assert, 2);

      /// @brief A Theia runtime assertion failed due to an unexpected NTSTATUS.
      ///
      /// Param1: Hash of the source filename.
      /// Param2: Line number (bottom 32 bits); optional tag identifying the assert (top 32 bits)
      /// Param3: The unexpected NTSTATUS.
      constexpr const NTSTATUS FailedNT = MakeCode(CrashCodeCategory::Assert, 3);

      /// @brief A Theia runtime assertion failed due to an unexpected Win32 status.
      ///
      /// Param1: Hash of the source filename.
      /// Param2: Line number (bottom 32 bits); optional tag identifying the assert (top 32 bits)
      /// Param3: The unexpected Win32 status.
      constexpr const NTSTATUS FailedWin32 = MakeCode(CrashCodeCategory::Assert, 4);

      /// @brief A developer triggered a remote crash.
      constexpr const NTSTATUS RemoteCrash = MakeCode(CrashCodeCategory::Assert, 5);

      /// @brief The theia::Interface::CrashSoft method was invoked.
      ///
      /// Param1: RIP that invoked the crash.
      /// Param2: Crashing module identifier.
      /// Param3: User provided.
      constexpr const NTSTATUS User_Induced_Soft = MakeCode(CrashCodeCategory::Assert, 6);

      /// @brief The theia::Interface::CrashHard method was invoked.
      ///
      /// Param1: RIP that invoked the crash.
      /// Param2: Crashing module identifier.
      /// Param3: User provided.
      constexpr const NTSTATUS User_Induced_Hard = MakeCode(CrashCodeCategory::Assert, 7);
    } // namespace Assert

    /// @brief Crash codes caused by various virtual machine and hypervisor detections.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace AntiVM {
      /// @brief Certain CPU registers did not validate, indicating the presence of a CPU emulator.
      constexpr const NTSTATUS Emulation = MakeCode(CrashCodeCategory::AntiVM, 1);

      /// @brief The brand of the current CPU is a known virtual machine/emulator brand.
      constexpr const NTSTATUS CPUIDBrand = MakeCode(CrashCodeCategory::AntiVM, 2);

      /// @brief The CPU indicates that a non-Hyper-V hypervisor is present.
      constexpr const NTSTATUS CPUIDHypervisor = MakeCode(CrashCodeCategory::AntiVM, 3);

      /// @brief The CPU responded to a backdoor instruction commonly used by hypervisors.
      constexpr const NTSTATUS CPUIDBackdoor = MakeCode(CrashCodeCategory::AntiVM, 4);

      /// @brief The BIOS vendor contains a known virtual machine/emulator brand.
      constexpr const NTSTATUS BIOSVendor = MakeCode(CrashCodeCategory::AntiVM, 5);

      /// @brief The system firmware contains a known virtual machine/emulator brand.
      constexpr const NTSTATUS SystemFirmware = MakeCode(CrashCodeCategory::AntiVM, 6);

      /// @brief The system SMBIOS contains a known virtual machine/emulator brand.
      constexpr const NTSTATUS SMBIOS = MakeCode(CrashCodeCategory::AntiVM, 7);

      /// @brief A driver belonging to a known virtual machine/emulator brand is loaded.
      constexpr const NTSTATUS VMDriver = MakeCode(CrashCodeCategory::AntiVM, 8);

      /// @brief A hardware device identifier contains is a known virtual machine/emulator brand.
      constexpr const NTSTATUS DeviceIdentifier = MakeCode(CrashCodeCategory::AntiVM, 9);

      /// @brief The machine identifies itself as running Hyper-V, but certain core Hyper-V services are not present.
      constexpr const NTSTATUS InvalidHyperV = MakeCode(CrashCodeCategory::AntiVM, 10);

      /// @brief The backdoor used by VMWare to identify itself is present.
      constexpr const NTSTATUS VMWareBackdoor = MakeCode(CrashCodeCategory::AntiVM, 11);

      /// @brief An uncommon CPU instruction was not emulated properly, indicating a CPU emulator or hypervisor.
      constexpr const NTSTATUS InvalidInstructionExecution = MakeCode(CrashCodeCategory::AntiVM, 12);
    } // namespace AntiVM

    /// @brief Crash codes caused by various anti-debugger and anti-anti-debugger detections.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace AntiDebug {
      /// @brief A debugger was detected through the PEB.
      constexpr const NTSTATUS PEB = MakeCode(CrashCodeCategory::AntiDebug, 1);

      /// @brief A debugger was detected because the process has a debug heap.
      ///
      /// Param1: Reserved.
      /// Param2: Value of HeapFlags.
      /// Param3: Value of HeapForceFlags.
      constexpr const NTSTATUS DebugHeap = MakeCode(CrashCodeCategory::AntiDebug, 2);

      /// @brief A debugger was detected because a debug-only exception was raised.
      constexpr const NTSTATUS DebugOnlyExceptionRaised = MakeCode(CrashCodeCategory::AntiDebug, 3);

      /// @brief A debugger was detected because a debug port was present.
      constexpr const NTSTATUS DebugPort = MakeCode(CrashCodeCategory::AntiDebug, 4);

      /// @brief The system is reporting that a kernel debugger is present.
      constexpr const NTSTATUS KernelDebuggerReported = MakeCode(CrashCodeCategory::AntiDebug, 5);

      /// @brief The system is improperly handling or ignoring common anti-debug checks,
      ///        indicating that an anti-anti-debug tool is present.
      constexpr const NTSTATUS AntiAntiDebug = MakeCode(CrashCodeCategory::AntiDebug, 6);
    } // namespace AntiDebug

    /// @brief Crash codes caused by an insecure machine configuration.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace MachineIntegrity {
      /// @brief The system reports that code-signing integrity is not properly enforced on this machine.
      constexpr const NTSTATUS SystemCodeIntegrity = MakeCode(CrashCodeCategory::MachineIntegrity, 1);

      /// @brief The system reports that secure boot is off.
      /// @note Only raised on Windows 11 and newer.
      constexpr const NTSTATUS NoSecureBoot = MakeCode(CrashCodeCategory::MachineIntegrity, 2);

      /// @brief The system launch options indicate that test or debug mode is turned on.
      constexpr const NTSTATUS SystemStartOptions = MakeCode(CrashCodeCategory::MachineIntegrity, 3);

      /// @brief A kernel-mode driver that is known to have vulnerabilities is loaded.
      constexpr const NTSTATUS VulnerableDriver = MakeCode(CrashCodeCategory::MachineIntegrity, 4);

      /// @brief The system reports that no TPM is available.
      /// @note Only raised on Windows 11 and newer.
      constexpr const NTSTATUS NoTPM = MakeCode(CrashCodeCategory::MachineIntegrity, 5);
    }

    /// @brief Crash codes caused by integrity failures within protected modules or the Theia runtime.
    ///
    /// @warning Code integrity failures will not dispatch crash callbacks since the integrity of the
    ///          crash callback dispatching mechanism can no longer be guaranteed. Any condition leading
    ///          to a code integrity failure will result in an immediate soft crash. As a result, the codes
    ///          documented in this category will only be found in crash dumps, and not in crash callbacks.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace CodeIntegrity {
      /// @brief The contents of a page within a protected module do not match the expected hash.
      ///
      /// Param1: Module identifier.
      /// Param2: Page index.
      /// Param3: Reflected mapping.
      constexpr const NTSTATUS HashMismatch = MakeCode(CrashCodeCategory::CodeIntegrity, 1);

      /// @brief Integrity checks of the Theia runtime failed.
      constexpr const NTSTATUS RuntimeIntegrity = MakeCode(CrashCodeCategory::CodeIntegrity, 2);

      /// @brief A heap allocation used by the Theia runtime has failed integrity checks.
      ///
      /// Param1: Address of the protected range.
      /// Param2: Size of the protected range.
      /// Param3: Tag identifying purpose of the range.
      constexpr const NTSTATUS RangeIntegrity = MakeCode(CrashCodeCategory::CodeIntegrity, 3);
    }

    /// @brief Crash codes caused by insecure elements within the process.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace ProcessIntegrity {
      /// @brief The process was suspended.
      ///
      /// Param1: Reserved.
      /// Param2: Time spent suspended.
      constexpr const NTSTATUS Suspended = MakeCode(CrashCodeCategory::ProcessIntegrity, 1);

      /// @brief Anti-cheat enforcement was configured, but no anti-cheat was detected.
      ///
      /// @note This crash code is only raised if an anti-cheat was detected during startup,
      ///       but somehow disappeared after startup. If no anti-cheat product was found during
      ///       startup, the process is terminated with the configured exit code instead of
      ///       raising this crash code.
      constexpr const NTSTATUS NoAntiCheat = MakeCode(CrashCodeCategory::ProcessIntegrity, 2);

      /// @brief The process had a console attached to it.
      constexpr const NTSTATUS Console = MakeCode(CrashCodeCategory::ProcessIntegrity, 3);

      /// @brief Protected memory was accessed outside critical section.
      ///
      /// Param1: Pointer to heap whose integrity was violated.
      /// Param2: Reserved
      /// Param3: Reserved
      constexpr const NTSTATUS ExternalMemoryAccess = MakeCode(CrashCodeCategory::ProcessIntegrity, 4);

      /// @brief Executable memory not backed by a module was detected.
      ///
      /// Param1 = Address of the executable memory.
      /// Param2 = Size of the executable memory.
      /// Param3 = Unused.
      constexpr const NTSTATUS FloatingExecutableMemory = MakeCode(CrashCodeCategory::ProcessIntegrity, 5);

      /// @brief An unsigned process with a handle open to our process was found.
      ///
      /// Param1 = Process ID.
      /// Param2 = Unused.
      /// Param3 = Unused.
      constexpr const NTSTATUS RemoteProcessHandle = MakeCode(CrashCodeCategory::ProcessIntegrity, 6);
    }

    /// @brief Crash codes caused by Theia's periodic task system being interrupted or failing.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace Periodic {
      /// @brief A periodic job did not complete in time.
      ///
      /// Param1: Functionality identifier.
      /// Param2: Last timestamp.
      constexpr const NTSTATUS Timeout = MakeCode(CrashCodeCategory::Periodic, 1);

      /// @brief A periodic job reported a time from the future.
      ///
      /// Param1: Functionality identifier.
      /// Param2: Future timestamp.
      constexpr const NTSTATUS Future = MakeCode(CrashCodeCategory::Periodic, 2);

      /// @brief A periodic job's structure was corrupted.
      ///
      /// Param1: Functionality identifier.
      /// Param2: Pointer to corrupted structure.
      constexpr const NTSTATUS Corruption = MakeCode(CrashCodeCategory::Periodic, 3);
    }

    /// @brief Crash codes related to single instance enforcement.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace SingleInstance {
      /// @brief A second instance of the same protected binary was detected.
      ///
      /// @note Launching a second instance while another is already running will simply cause
      ///       the second instance to immediately exit. Such a graceful exit will not invoke
      ///       a crash callback. Only if this graceful exit fails (or is bypassed by an
      ///       attacker) will this crash callback be invoked.
      ///
      /// @note It is not guaranteed in which process these detections will be raised. If more
      ///       than one process exists, this callback may be delivered in either the newest
      ///       process, the older one, or both.
      constexpr const NTSTATUS Detected = MakeCode(CrashCodeCategory::SingleInstance, 1);
    }

    /// @brief Crash codes related to badware detection.
    ///
    /// Crash codes within this category are identified by the badware product, and not the
    /// detection measure. This allows you to easily handle crash callbacks for a specific
    /// product. If you would like to respond to any detection of enabled badware, you can
    /// instead subscribe to the badware category and use GetCrashIndex(code) to derive the
    /// specific product that was detected.
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace Badware {
      /// https://www.virtualbox.org/
      constexpr const NTSTATUS VirtualBox = MakeCode(CrashCodeCategory::Badware, 1);

      /// https://www.vmware.com/products/workstation-player.html
      /// https://www.vmware.com/products/workstation-pro.html
      constexpr const NTSTATUS VMware = MakeCode(CrashCodeCategory::Badware, 2);

      /// https://support.microsoft.com/en-us/topic/description-of-windows-virtual-pc-262c8961-90e5-1125-654f-d87cd5ba16f8
      constexpr const NTSTATUS VirtualPC = MakeCode(CrashCodeCategory::Badware, 3);

      /// https://www.parallels.com/
      constexpr const NTSTATUS Parallels = MakeCode(CrashCodeCategory::Badware, 4);

      /// https://xenserver.org/
      constexpr const NTSTATUS XenServer = MakeCode(CrashCodeCategory::Badware, 5);

      /// https://www.qemu.org/
      constexpr const NTSTATUS QEMU = MakeCode(CrashCodeCategory::Badware, 6);

      /// https://www.ollydbg.de/
      constexpr const NTSTATUS OllyDbg = MakeCode(CrashCodeCategory::Badware, 7);

      /// https://x64dbg.com/
      constexpr const NTSTATUS x64dbg = MakeCode(CrashCodeCategory::Badware, 8);

      /// https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools
      /// https://apps.microsoft.com/store/detail/windbg-preview/9PGJGD53TN86?hl=en-us&gl=us
      constexpr const NTSTATUS WinDbg = MakeCode(CrashCodeCategory::Badware, 9);

      /// https://cheatengine.org/
      constexpr const NTSTATUS CheatEngine = MakeCode(CrashCodeCategory::Badware, 10);

      /// https://lldb.llvm.org/
      constexpr const NTSTATUS LLDB = MakeCode(CrashCodeCategory::Badware, 11);

      /// https://github.com/IChooseYou/Reclass64
      /// https://github.com/ajkhoury/ReClassEx
      /// https://github.com/ReClassNET/ReClass.NET
      constexpr const NTSTATUS ReClass = MakeCode(CrashCodeCategory::Badware, 12);

      /// https://learn.microsoft.com/en-us/sysinternals/downloads/procmon
      constexpr const NTSTATUS ProcMon = MakeCode(CrashCodeCategory::Badware, 13);

      /// https://www.wireshark.org/
      constexpr const NTSTATUS Wireshark = MakeCode(CrashCodeCategory::Badware, 14);

      /// https://hex-rays.com/IDA-pro/
      constexpr const NTSTATUS IDA = MakeCode(CrashCodeCategory::Badware, 15);

      /// https://ntcore.com/?page_id=388
      constexpr const NTSTATUS CFFExplorer = MakeCode(CrashCodeCategory::Badware, 16);

      /// https://github.com/hasherezade/pe-bear
      constexpr const NTSTATUS PEBear = MakeCode(CrashCodeCategory::Badware, 17);

      /// https://github.com/hfiref0x/WinObjEx64
      constexpr const NTSTATUS WinObjEx = MakeCode(CrashCodeCategory::Badware, 18);

      /// https://github.com/DarthTon/Xenos
      constexpr const NTSTATUS Xenos = MakeCode(CrashCodeCategory::Badware, 19);

      /// https://github.com/KANKOSHEV/face-injector-v2
      constexpr const NTSTATUS FaceInjector = MakeCode(CrashCodeCategory::Badware, 20);

      /// https://guidedhacking.com/
      constexpr const NTSTATUS GHInjector = MakeCode(CrashCodeCategory::Badware, 21);

      /// https://github.com/everXenia/SetWindowsHookEx-Injector
      constexpr const NTSTATUS HookLoader = MakeCode(CrashCodeCategory::Badware, 22);

      /// https://github.com/manovisible/lunarinjector
      constexpr const NTSTATUS LunarInjector = MakeCode(CrashCodeCategory::Badware, 23);

      // https://guidedhacking.com/
      constexpr const NTSTATUS GHBasicInjector = MakeCode(CrashCodeCategory::Badware, 24);

      /// https://www.unknowncheats.me/forum/general-programming-and-reversing/209939-alisaalis-manual-map-dll-injector-x86-x64.html
      constexpr const NTSTATUS AlisInjector = MakeCode(CrashCodeCategory::Badware, 25);

      /// https://github.com/master131/ExtremeInjector
      constexpr const NTSTATUS ExtremeInjector = MakeCode(CrashCodeCategory::Badware, 26);

      /// https://www.unknowncheats.me/forum/warface/153457-windows-10-injector.html
      constexpr const NTSTATUS Windows10Injector = MakeCode(CrashCodeCategory::Badware, 27);

      /// Any running process with [unknowncheats.me] in its name.
      constexpr const NTSTATUS UnknowncheatsGeneric = MakeCode(CrashCodeCategory::Badware, 28);

      /// https://learn.microsoft.com/en-us/windows-hardware/drivers/devtest/dtrace
      constexpr const NTSTATUS DTrace = MakeCode(CrashCodeCategory::Badware, 29);

      /// https://processhacker.sourceforge.io/
      constexpr const NTSTATUS ProcessHacker = MakeCode(CrashCodeCategory::Badware, 30);

      /// https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer
      constexpr const NTSTATUS ProcExp = MakeCode(CrashCodeCategory::Badware, 31);

      /// https://npcap.com/
      constexpr const NTSTATUS Npcap = MakeCode(CrashCodeCategory::Badware, 32);

      /// http://memoryhacking.com/
      /// https://github.com/L-Spiro/MhsX
      constexpr const NTSTATUS MHS = MakeCode(CrashCodeCategory::Badware, 33);

      /// https://www.cheaters-heaven.com/games/tools/35171-tsearch-v1-6.html
      constexpr const NTSTATUS TSearch = MakeCode(CrashCodeCategory::Badware, 35);

      /// https://www.artmoney.ru/
      constexpr const NTSTATUS ArtMoney = MakeCode(CrashCodeCategory::Badware, 36);

      /// https://www.unknowncheats.me/forum/general-programming-and-reversing/295252-cheat-tool-set-tool-ce-reclass.html
      constexpr const NTSTATUS CheatToolSet = MakeCode(CrashCodeCategory::Badware, 37);

      /// https://www.squalr.com/
      constexpr const NTSTATUS Squalr = MakeCode(CrashCodeCategory::Badware, 38);

      // https://www.autohotkey.com/
      constexpr const NTSTATUS AutoHotkey = MakeCode(CrashCodeCategory::Badware, 39);

      // https://www.rewasd.com/
      constexpr const NTSTATUS ReWASD = MakeCode(CrashCodeCategory::Badware, 40);
    } // namespace Badware

    /// @brief Crash codes related to hardware error.
    ///
    /// Crash codes here are strong indicators of hardware failure. Note that lack of them
    /// does not mean lack of faulty hardware, and extremely rare occurrence does not mean presence
    /// of it, as this can also be caused by cosmic rays or other ionizing radiation particles
    /// (this is not a joke: https://en.wikipedia.org/wiki/Soft_error).
    ///
    /// @warning The set of crashes within this category is not guaranteed to be exhaustive.
    namespace HardwareError {
      /// @brief Hashing the same data twice resulted in different results.
      ///
      /// Param1: Expected.
      /// Param2: Result 1.
      /// Param3: Result 2.
      constexpr const NTSTATUS HashTwice = MakeCode(CrashCodeCategory::HardwareError, 1);
    }
  } // namespace CrashCode

  /// @brief A general category of telemetry callbacks.
  ///
  /// Telemetry callbacks are similar to crash callbacks, but for situations that are non-fatal.
  /// They exist to inform you that Theia took some action, but do not give you the option to
  /// influence this action. They generally are useful to communicate information to the current
  /// user, or to report to your backend for diagnosing potential issues.
  ///
  /// As with crash callbacks, you are able to subscribe to telemetry callbacks for an entire category.
  /// When done so, you will receive crash callbacks for all possible telemetry notifications within
  /// that category. The NTSTATUS parameter passed to the callback will identify the exact purpose
  /// of the notification, and can be extracted using theia::TelemetryCode::GetCategory.
  ///
  /// @warning This enumeration is not exhaustive. Newer versions of the Theia runtime may report
  ///          values outside this enumeration without a major version update.
  enum class TelemetryCodeCategory : uint8_t {
    /// @brief Invalid.
    Invalid = 0,

    /// @brief Telemetry notifications related to the enforcement of code signing within the process.
    CodeSigning,

    /// @brief Telemetry notifications related to the enforcement of process integrity.
    ProcessIntegrity,
  };

  /// @brief Facilities relating to Theia telemetry codes.
  ///
  /// As with crash codes, telemetry codes are constructed as custom NTSTATUS values. You can use the methods
  /// exposed in this namespace to introspect such statuses. Individual sub-namespaces of this namespace expose
  /// individual telemetry codes.
  ///
  /// The `Code` part of a Theia telemetry NTSTATUS consists of a 16-bit value. The top 8 bits represent the
  /// telemetry code category, whereas the bottom 8 bits represent a unique notification within that category.
  /// Note that the unique index `0` is a reserved value and will never be reported.
  ///
  /// As with crash callbacks, three parameters are passed to telemetry callbacks that represent additional
  /// information related to the telemetry callback. Unless specifically documented, these parameters contain
  /// reserved values and you should not depend on their value. Nevertheless, it is recommended that you store
  /// and/or report these values to a backend, as Zero IT Lab may be able to use their undocumented contents
  /// to help you diagnose issues.
  ///
  /// @warning The list of diagnostic codes exposed in this namespace is not exhaustive. Newer versions of
  ///          the Theia runtime may issue new diagnostic callbacks not listed in this namespace without a
  ///          major version update. Your code must be able to handle unknown status codes.
  namespace TelemetryCode {
    /// @brief Symbolic constant that can be used to subscribe to all telemetry callbacks.
    constexpr const detail::AllTelemetrySentinel All{};

    /// @brief Check whether the given NTSTATUS is a custom Theia telemetry status.
    constexpr bool IsTheiaTelemetryCode(NTSTATUS status) {
      bool isCustom = ((status >> 29) & 1) == 1;
      bool isTheiaFacility = ((status >> 16) & 0xFFF) == STATUS_FACILITY_THEIA;
      bool isInformationalSeverity = ((status >> 30) & 3) == 1;
      return isCustom && isInformationalSeverity && isTheiaFacility;
    }

    /// @brief Build a Theia telemetry status from the given category and index.
    constexpr NTSTATUS MakeCode(TelemetryCodeCategory category, uint8_t index) {
      auto combined = (uint16_t)((((uint16_t)category) << 8) | (uint16_t)index);
      return (NTSTATUS)((uint32_t)0
                        | (((uint32_t)1) << 30)                               // STATUS_SEVERITY_INFORMATIONAL
                        | (((uint32_t)1) << 29)                               // CUSTOM
                        | ((((uint32_t)STATUS_FACILITY_THEIA) & 0xFFF) << 16) // FACILITY
                        | ((((uint32_t)combined) & 0xFFFF)));                 // CODE
    }

    /// @brief Retrieve the telemetry code category from the given status. Return
    ///        TelemetryCodeCategory::Invalid if not a Theia crash code.
    constexpr TelemetryCodeCategory GetTelemetryCategory(NTSTATUS status) {
      if (!IsTheiaTelemetryCode(status))
        return TelemetryCodeCategory::Invalid;

      auto value = (uint16_t)(status & 0xFFFF);
      return static_cast<TelemetryCodeCategory>(value >> 8);
    }

    /// @brief Retrieve the unique notification index for the given Theia telemetry code.
    ///        Return zero if the input is not a valid Theia telemetry code.
    constexpr uint8_t GetTelemetryNotificationIndex(NTSTATUS status) {
      if (!IsTheiaTelemetryCode(status))
        return 0;

      return (uint8_t)(status & 0xFF);
    }

    /// @brief Telemetry codes related to code-signing enforcement.
    namespace CodeSigning {
      /// @brief An unsigned DLL attempted to load into the process, but it was prevented.
      ///
      /// @note This telemetry callback is only emitted if code signature enforcement is enabled.
      ///
      /// @note This telemetry callback will only be invoked for DLLs that are loading into the
      ///       process after the process has already initialized. If one of the dependencies of
      ///       a protected module has an invalid or missing code signature, the entire module will
      ///       fail to load.
      ///
      /// @warning The string pointed to by `Param1` is not guaranteed to persist beyond the
      ///          lifetime of this call. Make a copy if you want to refer to the contents later.
      ///
      /// Param1: A PWSTR pointer to a utf-16 null-terminated path of the rejected DLL.
      constexpr const NTSTATUS RejectedUnsignedDLL = MakeCode(TelemetryCodeCategory::CodeSigning, 1);
    }

    /// @brief Telemetry notifications related to the enforcement of process integrity.
    namespace ProcessIntegrity {
      /// @brief Executable memory not backed by a module was detected and marked as non-executable.
      ///
      /// Param1: Address of the executable memory.
      /// Param2: Size of the executable memory.
      constexpr const NTSTATUS FloatingExecutableMemory = MakeCode(TelemetryCodeCategory::ProcessIntegrity, 1);
    }
  } // namespace TelemetryCode

  /// @brief Return value of a crash callback. Determines what the Theia runtime will do next.
  enum class SDKCallbackAction {
    /// @brief Continue calling other handlers (if any), or do the
    ///        default action if this was the last available handler.
    Default,

    /// @brief Perform a hard crash.
    Crash,

    /// @brief Continue execution normally, ignore the crash.
    ///
    /// @note Some crash callbacks do not support this behavior and will always crash. For
    ///       these callbacks, this behavior is explicitly noted in their documentation.
    ///
    /// @note Returning `Continue` only allows the execution for the current crash to
    ///       continue. Detections for the given crash will continue to run and may
    ///       result in a new crash callback with the same parameters to be emitted later.
    Continue
  };

  /// @brief Function type signature for crash callbacks that return an SDKCallbackAction.
  ///
  /// @note If you will always return `Default`, consider using a NotifyCallbackType instead.
  using CrashCallbackType = SDKCallbackAction(NTSTATUS, uintptr_t, uintptr_t, uintptr_t);

  /// @brief Function type signature for crash callbacks that do not return any actions.
  ///
  /// The benefit of using this over a CrashCallbackType is that the compiled Theia runtime will
  /// statically know the result of the crash, allowing it to produce more efficient code and
  /// avoiding the possibility of an attacker hooking your crash callback and returning a value
  /// of SDKCallbackAction::Continue.
  ///
  /// @note If you want to influence the behavior of the crash, use CrashCallbackType instead.
  using NotifyCallbackType = void(NTSTATUS, uintptr_t, uintptr_t, uintptr_t);

  /// @brief Function type signature for telemetry callbacks.
  using TelemetryCallbackType = void(NTSTATUS, uintptr_t, uintptr_t, uintptr_t);

  /// @brief Register a callback for a crash or telemetry callback.
  ///
  /// @param Function The function to invoke when the specific callback is emitted. This function must
  ///                 be of the appropriate type, either  CrashCallbackType, NotifyCallbackType, or
  ///                 TelemetryCallbackType.
  /// @param ForWhat The notification to register for. Can be either a CrashCodeCategory, a TelemetryCodeCategory,
  ///                or the specific NTSTATUS itself.
#define THEIA_REGISTER_CALLBACK(Function, ForWhat) THEIA_REGISTER_CALLBACK_IMPL(THEIA_CONCAT(_theia_CT, __LINE__), Function, ForWhat)

  /// @brief Statistics on the protected pages in the current module.
  struct PageStats {
    /// @brief Size of this structure.
    uint32_t BufferSize = sizeof(PageStats);

    /// @brief Count of all pages in the module.
    uint32_t CountPagesTotal{};

    /// @brief Count of all pages in the module that are currently decrypted.
    uint32_t CountPagesDecrypted{};

    /// @brief Count of total eligible pages in the module for re-encryption.
    uint32_t CountPagesEncryptableTotal{};

    /// @brief Count of eligible pages in the module for re-encryption that are currently decrypted.
    uint32_t CountPagesEncryptableDecrypted{};
  };

  /// @brief A Theia protected heap instance.
  ///
  /// The Theia protected heap is suitable for allocations of small to mid-sized objects whose usage
  /// can be captured in critical sections. When leaving the critical section, Theia will mark all
  /// allocated memory as unreachable. Any access to this memory outside the critical section will
  /// be logged, and a crash will be issued the next time the critical section is entered.
  ///
  /// The protected heap satisfies the C++ @c BasicLockable concept, and as a result can be used in
  /// <atomic> types such as std::lock_guard. This is the recommended way to enforce critical sections.
  ///
  /// @note The Heap is not re-entrant: attempting to lock the heap from within a critical section is
  ///       undefined behavior and may result in a deadlock. Calls to `lock` must eventually be followed
  ///       by a matching `unlock` call, otherwise behavior is undefined.
  ///
  /// @note The Heap does not contain any synchronization primitives: it is the responsibility of the
  ///       caller to ensure that no other thread is accessing the heap at the same time.
  ///
  /// @note The heap must be destroyed with `Destroy`, and not `delete`. Failure to do so will result
  ///       in a memory leak. Once `Destroy` is called, further access to the heap is undefined as the
  ///       backing memory may have been freed.
  ///
  /// @note Memory access outside the critical section will also be detected from sources outside the
  ///       process, including other processes and the kernel itself. Because such memory access can
  ///       come from legitimate software (e.g. malware scanning tools), Theia uses a heuristic approach
  ///       based on the time and frequency of such accesses to determine whether to issue a crash.
  class Heap {
  public:
    /// @brief Destroy the heap and release all backing memory.
    ///
    /// This function will free all remaining allocations and the backing control structures, and
    /// must be called instead of the `delete` operator to delete a Heap instance. After calling,
    /// any further access to the heap and any allocations made from it will result in undefined
    /// behavior.
    virtual void Destroy() noexcept = 0;

    /// @brief Allocate a memory region of the given size.
    ///
    /// If `Size` is zero, some implementation defined value is returned. This value may or may not
    /// be a null pointer, but is always legal to pass to `DeAllocate` and `ReAllocate`.
    ///
    /// The returned value has an alignment of double pointer size, i.e. 16 bytes. If you require
    /// a bigger alignment, you must manually align the pointer yourself.
    ///
    /// @note Calling this function outside of a critical section is undefined behavior.
    /// @param Size Amount of bytes required for newly allocated storage.
    /// @return pointer of newly allocated space or @c nullptr if allocation failed.
    virtual void* Allocate(size_t Size) = 0;

    /// @brief Deallocate a previously allocated object.
    ///
    /// Calling this function with a pointer not allocated by this heap, or a pointer that
    /// has already been deallocated, is undefined behavior. Further access to the memory
    /// region after calling this function is undefined behavior.
    ///
    /// @note Calling this function outside of a critical section is undefined behavior.
    /// @param Pointer Pointer to deallocate.
    /// @return @c true if deallocation succeeded, @c false otherwise.
    virtual bool DeAllocate(void* Pointer) = 0;

    /// @brief Enter the critical section and allow access to the heap and its memory.
    ///
    /// Enters the critical section for this heap and mark its memory as available for usage
    /// in this thread. Detect if any access to the heap has occurred since the last call to
    /// @c unlock, and potentially raise a crash if so.
    ///
    /// @warning This is **NOT** a synchronization primitive: the function is named `lock` for
    ///          consistency with the @c BasicLockable concept, but it does not guarantee mutual
    ///          exclusion. It is the responsibility of the caller to ensure that no other thread
    ///          is accessing the heap at the same time.
    ///
    /// @note This name conforms with the @c BasicLockable concept for usage with @c std::lock_guard.
    /// @note Calls to this function must always be paired with a call to @c unlock.
    /// @note This function is not re-entrant: calling this function from within a critical section
    ///       is undefined behavior and may result in a deadlock.
    virtual void lock() = 0;

    /// @brief Leave the critical section and disallow further access to its memory.
    ///
    /// Any further access to the heap and its allocations will be traced and potentially result
    /// in a crash the next time @c lock is called.
    ///
    /// @warning This is **NOT** a synchronization primitive: the function is named `unlock` for
    ///          consistency with the @c BasicLockable concept, but it does not guarantee mutual
    ///          exclusion. It is the responsibility of the caller to ensure that no other thread
    ///          is accessing the heap at the same time.
    ///
    /// @note This name conforms with the @c BasicLockable concept for usage with @c std::lock_guard.
    /// @note Calling this function outside of a critical section is undefined behavior.
    virtual void unlock() = 0;

  protected:
    /// @brief This function is private because interfaces are not supposed to be constructed.
    Heap() = default;

    /// @brief This function is deleted because interfaces are not supposed to be copy-constructed.
    Heap(const Heap&) = delete;

    /// @brief This function is deleted because interfaces are not supposed to be move-constructed.
    Heap(Heap&&) = delete;

    /// @brief This function is deleted because interfaces are not supposed to be copy-assigned.
    Heap& operator=(const Heap&) = delete;

    /// @brief This function is deleted because interfaces are not supposed to be move-assigned.
    Heap& operator=(Heap&&) = delete;

    /// @brief Heap objects can not be destroyed by normal means because they are responsible for management
    ///        of their own memory. Use @c Destroy instead.
    virtual ~Heap() noexcept = 0;

  public:
    /// @brief Returns the implementation specific handle representing the backing heap.
    ///
    /// For heaps in a protected (i.e. packed) process, this will return a handle suitable
    /// for use in the Win32 heap APIs: https://learn.microsoft.com/en-us/windows/win32/api/heapapi/.
    ///
    /// In unprotected processes, and on platforms where a full implementation of the protected
    /// heap is not available, this function will return @c nullptr.
    ///
    /// @note Using this handle while outside the critical section may result in undefined behavior.
    /// @return Native handle for backing heap if any, otherwise @c nullptr.
    virtual void* GetBackingHeap() = 0;

    /// @brief Attempt to reallocate the given block of memory.
    ///
    /// Allocate a new block of memory with the given `Size`, and copy existing contents of
    /// `Pointer` to the new block. If possible, the implementation will extend the existing
    /// memory instead of allocating a new block. The resulting pointer is guaranteed to be
    /// aligned to double pointer size, i.e. 16 bytes.
    ///
    /// If `Pointer` is @c nullptr, this function behaves as if @c Allocate was called with
    /// the given size. If `Size` is zero, the behavior is as if the size were some unspecified
    /// nonzero value.
    ///
    /// If `Pointer` was not allocated by this heap, or if it has already been deallocated,
    /// the behavior is undefined.
    ///
    /// @note Calling this function outside of a critical section is undefined behavior.
    /// @param Pointer Pointer to previously allocated memory or @c nullptr for new allocation.
    /// @param Size Size of new allocation.
    /// @return Pointer to allocated memory or @c nullptr only if reallocation failed.
    virtual void* ReAllocate(void* Pointer, size_t Size) = 0;

    /// @brief Retrieve the size of the given allocation.
    ///
    /// If `Pointer` was not allocated by this heap, or if it has already been deallocated,
    /// the behavior is undefined.
    ///
    /// Not all implementations may support this functionality. If not supported, this function
    /// will always return @c -1.
    ///
    /// @note Calling this function outside of a critical section is undefined behavior.
    /// @return Allocation size of `Pointer` if successful, @c -1 otherwise.
    virtual size_t GetSize(void* Pointer) = 0;
  };

  /// @brief Interface to the Theia runtime exposed to your protected binary.
  ///
  /// You can use this interface to communicate with the Theia runtime. To obtain an interface
  /// for the current module, invoke the GetInterface() function. If you are not running under
  /// a protected runtime, the implementations of the functions will contain dummy values.
  struct FunctionPtrs {
    /// @brief Signal that the module is ready to receive callbacks.
    ///
    /// Any callbacks emitted by the Theia runtime before this function is called will
    /// resolve to their default value. To avoid crashing without receiving callbacks,
    /// ensure that this function is called early in your program's initialization.
    void (*ReadyForCallbacks)();

    /// @brief Crash the process with CrashCode::Assert::User_Induced_Soft.
    ///
    /// @param Param3 Third crash parameter.
    void (*CrashSoft)(uintptr_t Param3);

    /// @brief Crash the process with CrashCode::Assert::User_Induced_Hard.
    ///
    /// @param Param3 Third crash parameter.
    void (*CrashHard)(uintptr_t Param3);

    /// @brief Return if this executable was protected.
    ///
    /// @return True if protected.
    bool (*IsProtected)();

    /// @brief Return if the current call stack is valid.
    ///
    /// Callstack validation often fails if manually injected code is on the call stack. However it also fails if
    /// someone wrongly implements unwinding (for example OpenSSL), which can be diagnosed by @c LastValid.
    ///
    /// @param InvalidRetaddr The invalid return address, if validation failed.
    /// @param LastValid The last valid return address, if validation failed.
    /// @return True if the callstack was valid, false otherwise.
    bool (*ValidateCallstack)(uintptr_t& InvalidRetaddr, uintptr_t& LastValid);

    /// @brief Instruct the Theia runtime to re-encrypt parts of the executable.
    ///
    /// When invoked, re-encrypts parts of the executable. How much is re-encrypted depends on the parameter
    /// `Amount`.
    ///
    /// Currently, only a value of MAX_UINT (0xFFFFFFFF) is supported. This represents a full re-encryption of the
    /// binary, resulting in all eligible decrypted pages being re-encrypted.
    ///
    /// This function is thread-safe and can be called from multiple threads at the same time without issues.
    ///
    /// @param Amount Must be 0xFFFFFFFF.
    void (*Encrypt)(uint32_t Amount);

    /// @brief Retrieve statistics on protected pages for the current module.
    ///
    /// The buffer size field must be initialized properly with correct size of structure. This ensures that any
    /// additions to @c PageStats continue working with binaries built against older SDKs.
    ///
    /// @param OutputBuffer Pointer to output buffer.
    /// @return 0 if successful.
    size_t (*QueryPageStats)(PageStats* OutputBuffer);

    /// @brief Create a new protected heap that can be used to serve protected allocations.
    ///
    /// If the current module is unprotected (i.e. not packed), a per-module global instance is returned which
    /// serves allocations directly from the CRT allocator, and both `ReservedSize` and `MaxAllocSize` are not
    /// enforced.
    ///
    /// @note Attempting to allocate more than `MaxAllocSize` bytes in a single call to `Allocate` is undefined
    ///       behavior and may or may not yield a valid pointer.
    ///
    /// @note `ReservedSize` represents the total amount of memory that the entire protected heap reserves. This
    ///       includes internal bookkeeping structures and other overhead. As a result, the amount of memory actually
    ///       available for allocations will be marginally less than `ReservedSize`. Upon exhaustion, future allocations
    ///       will fail until memory is freed.
    ///
    /// @note The resulting heap must be destroyed using `Destroy` when no longer needed, even if all memory has
    ///       been freed. Failure to do so will result in a memory leak.
    ///
    /// @param ReservedSize Count of bytes to reserve for newly created heap.
    /// @param MaxAllocSize Count of bytes for maximum allowed allocation size or @c 0 for any size.
    /// @param Reserved Must be nullptr.
    /// @return Pointer to an instance of protected heap.
    Heap* (*CreateHeap)(size_t ReservedSize, size_t MaxAllocSize, void* Reserved);
  };

  /// @brief Get the Theia interface.
  /// @return Pointer to Theia functions, or placeholders if Theia is not applied.
  inline const FunctionPtrs* GetInterface() {
    return detail::g_TheiaVMT;
  }
} // namespace THEIA_REAL_NAMESPACE

// Internal default implementations for theia::FunctionPtrs, feel free to ignore.
namespace THEIA_REAL_NAMESPACE {
  namespace detail {
    template <typename P, typename = size_t>
    struct CallMSize {
      static size_t Call(void* /* Pointer */) {
        return (size_t)-1;
      }
    };

    template <typename P>
    struct CallMSize<P, decltype(_msize((P) nullptr))> {
      static size_t Call(void* Pointer) {
        return Pointer != nullptr ? _msize(Pointer) : 0;
      }
    };

    static void DefaultReadyForCallbacks() {
    }

    static void DefaultCrash(uintptr_t Param3) {
      (void)Param3;
      *reinterpret_cast<char*>(static_cast<intptr_t>(-1)) = 0;
    }

    static bool DefaultIsProtected() {
      return false;
    }

    static bool DefaultValidateCallstack(uintptr_t& InvalidRetaddr, uintptr_t& LastValid) {
      InvalidRetaddr = LastValid = 0;
      return true;
    }

    static void DefaultEncrypt(uint32_t Amount) {
      (void)Amount;
    }

    static size_t DefaultQueryPageStats(PageStats* OutputBuffer) {
      if (OutputBuffer->BufferSize == sizeof(PageStats)) {
        *OutputBuffer = {};
        return 0;
      }
      return (size_t)-1;
    }

    template <typename>
    static Heap* DefaultCreateHeap(size_t /* ReservedSize */, size_t /* MaxAllocSize */, void* /* Reserved */) {
      class Impl final : public Heap {
      public:
        constexpr Impl() = default;

        ~Impl() override = default;

        void Destroy() noexcept override {}

        void* Allocate(size_t Size) override {
          return std::malloc(Size);
        }

        bool DeAllocate(void* Pointer) override {
          std::free(Pointer);
          return true;
        }

        void lock() override {}

        void unlock() override {}

        void* GetBackingHeap() override {
          return nullptr;
        }

        void* ReAllocate(void* Pointer, size_t Size) override {
          return std::realloc(Pointer, Size != 0 ? Size : 1);
        }

        size_t GetSize(void* Pointer) override {
          return theia::detail::CallMSize<void*>::Call(Pointer);
        }
      };

      static Impl impl;
      return &impl;
    }
  } // namespace detail
} // namespace THEIA_REAL_NAMESPACE

namespace THEIA_REAL_NAMESPACE {
  /// @brief Insert this macro into exactly one object.
#define THEIA_ONCE()                                                                                                                    \
  static constexpr theia::FunctionPtrs s_FunctionPtrDefaults{                                                                           \
    &theia::detail::DefaultReadyForCallbacks,                                                                                           \
    &theia::detail::DefaultCrash,                                                                                                       \
    &theia::detail::DefaultCrash,                                                                                                       \
    &theia::detail::DefaultIsProtected,                                                                                                 \
    &theia::detail::DefaultValidateCallstack,                                                                                           \
    &theia::detail::DefaultEncrypt,                                                                                                     \
    &theia::detail::DefaultQueryPageStats,                                                                                              \
    &theia::detail::DefaultCreateHeap<void>,                                                                                            \
  };                                                                                                                                    \
  ::theia::Heap::~Heap() = default;                                                                                                     \
  THEIA_EXPORT const theia::FunctionPtrs* theia::detail::g_TheiaVMT = &s_FunctionPtrDefaults;                                           \
  template <>                                                                                                                           \
  THEIA_EXPORT void ::theia::detail::TheiaSDKIdentifier<::theia::THEIA_MAJOR_VERSION, ::theia::THEIA_INTERFACE_VERSION>::DoNotCall() {} \
  thread_local theia::detail::ForceDynamicInitializer theia::detail::g_ForceDynamicInitializer;                                         \
  THEIA_EXPORT THEIA_SECTION(".data") const uint8_t theia::detail::g_DataBlob[0x2000]{};                                                \
  THEIA_EXPORT THEIA_SECTION(".rdata") const uint8_t theia::detail::g_RdataBlob[0x2000]{};                                              \
  THEIA_EXPORT THEIA_SECTION(".text") const uint8_t theia::detail::g_TextBlob[0x2000]{};

#ifdef THEIA_NOP
#define THEIA_NOP4() \
  THEIA_NOP();       \
  THEIA_NOP();       \
  THEIA_NOP();       \
  THEIA_NOP()
#define THEIA_NOP16() \
  THEIA_NOP4();       \
  THEIA_NOP4();       \
  THEIA_NOP4();       \
  THEIA_NOP4()
#define THEIA_NOP64() \
  THEIA_NOP16();      \
  THEIA_NOP16();      \
  THEIA_NOP16();      \
  THEIA_NOP16()
#define THEIA_NOP256() \
  THEIA_NOP64();       \
  THEIA_NOP64();       \
  THEIA_NOP64();       \
  THEIA_NOP64()
#define THEIA_NOP1024() \
  THEIA_NOP256();       \
  THEIA_NOP256();       \
  THEIA_NOP256();       \
  THEIA_NOP256()
#define THEIA_NOP4096() \
  THEIA_NOP1024();      \
  THEIA_NOP1024();      \
  THEIA_NOP1024();      \
  THEIA_NOP1024()

  /// @brief Insert a guard page that must never be decrypted. Works best if multiple object files have it at the end.
#define THEIA_GUARD_PAGE()                                                                          \
  template <>                                                                                       \
  THEIA_EXPORT void ::theia::detail::PageGuardian<::theia::detail::Fnv64a(__FILE__)>::DoNotCall() { \
    THEIA_NOP4096();                                                                                \
    THEIA_NOP4096();                                                                                \
  }
#endif
} // namespace THEIA_REAL_NAMESPACE

// More internals, please collapse or ignore.
namespace THEIA_REAL_NAMESPACE {
  namespace detail {
    template <class>
    constexpr bool dependent_false = false; // workaround before CWG2518/P2593R1

    // whether the given crash code category can be subscribed to
    constexpr bool IsLegalSubscription(CrashCodeCategory cat) {
      return cat != CrashCodeCategory::CodeIntegrity;
    }

    template <typename Code, typename Fun, Code Value>
    struct CallbackTraits {
      static_assert(dependent_false<Fun>, "Invalid parameters passed to THEIA_REGISTER_CALLBACK. The callback subject must be of type CrashCodeCategory, TelemetryCodeCategory, or const NTSTATUS.");
    };

    template <typename Fun, CrashCodeCategory Value>
    struct CallbackTraits<CrashCodeCategory, Fun, Value> {
      static_assert(std::is_same<Fun, CrashCallbackType>::value || std::is_same<Fun, NotifyCallbackType>::value, "Crash callbacks registered for a category must be of type CrashCallbackType or NotifyCallbackType.");
      static_assert(IsLegalSubscription(Value), "CodeIntegrity crashes do not produce crash callbacks and cannot be subscribed to.");

      using return_type = typename std::conditional<std::is_same<Fun, CrashCallbackType>::value, SDKCallbackAction, void>::type;
      static constexpr const uint32_t flags = 0
                                              | THEIA_CALLBACK_FLAGS_CATEGORY
                                              | (std::is_same<Fun, NotifyCallbackType>::value ? THEIA_CALLBACK_FLAGS_NO_RETURN_VALUE : 0);
      static constexpr const uint32_t code = static_cast<uint32_t>(Value);
    };

    template <typename Fun, AllCrashSentinel Value>
    struct CallbackTraits<AllCrashSentinel, Fun, Value> {
      static_assert(std::is_same<Fun, CrashCallbackType>::value || std::is_same<Fun, NotifyCallbackType>::value, "Catch-all crash callbacks must be of type CrashCallbackType or NotifyCallbackType.");

      using return_type = typename std::conditional<std::is_same<Fun, CrashCallbackType>::value, SDKCallbackAction, void>::type;
      static constexpr const uint32_t flags = 0
                                              | (std::is_same<Fun, NotifyCallbackType>::value ? THEIA_CALLBACK_FLAGS_NO_RETURN_VALUE : 0);
      static constexpr const uint32_t code = 0;
    };

    template <typename Fun, TelemetryCodeCategory Value>
    struct CallbackTraits<TelemetryCodeCategory, Fun, Value> {
      static_assert(std::is_same<Fun, TelemetryCallbackType>::value, "Telemetry callbacks registered for a category must be of type TelemetryCallbackType.");

      using return_type = void;
      static constexpr const uint32_t flags = 0
                                              | THEIA_CALLBACK_FLAGS_TELEMETRY
                                              | THEIA_CALLBACK_FLAGS_CATEGORY
                                              | THEIA_CALLBACK_FLAGS_NO_RETURN_VALUE;
      static constexpr const uint32_t code = static_cast<uint32_t>(Value);
    };

    template <typename Fun, AllTelemetrySentinel Value>
    struct CallbackTraits<AllTelemetrySentinel, Fun, Value> {
      static_assert(std::is_same<Fun, TelemetryCallbackType>::value, "Catch-all telemetry callbacks must be of type TelemetryCallbackType.");

      using return_type = void;
      static constexpr const uint32_t flags = 0
                                              | THEIA_CALLBACK_FLAGS_TELEMETRY
                                              | THEIA_CALLBACK_FLAGS_NO_RETURN_VALUE;
      static constexpr const uint32_t code = 0;
    };

    template <typename Fun, NTSTATUS Value>
    struct CallbackTraits<NTSTATUS, Fun, Value> {
      static constexpr const bool is_crash_code = CrashCode::IsTheiaCrashCode(Value);
      static constexpr const bool is_telemetry_code = TelemetryCode::IsTheiaTelemetryCode(Value);

      static_assert(is_crash_code || is_telemetry_code, "The NTSTATUS passed to THEIA_REGISTER_CALLBACK must be a Theia crash or telemetry code.");

      static_assert(is_crash_code ? (std::is_same<Fun, CrashCallbackType>::value || std::is_same<Fun, NotifyCallbackType>::value) : true, "Crash callbacks must be of type CrashCallbackType or NotifyCallbackType.");
      static_assert(is_telemetry_code ? std::is_same<Fun, TelemetryCallbackType>::value : true, "Telemetry callbacks must be of type TelemetryCallbackType.");

      static_assert(is_crash_code ? IsLegalSubscription(theia::CrashCode::GetCrashCategory(Value)) : true, "CodeIntegrity crashes do not produce crash callbacks and cannot be subscribed to.");

      using return_type = typename std::conditional<std::is_same<Fun, CrashCallbackType>::value, SDKCallbackAction, void>::type;
      static constexpr const uint32_t flags = 0
                                              | (is_telemetry_code ? THEIA_CALLBACK_FLAGS_TELEMETRY : 0)
                                              | (!std::is_same<Fun, CrashCallbackType>::value ? THEIA_CALLBACK_FLAGS_NO_RETURN_VALUE : 0);
      static constexpr const uint32_t code = static_cast<uint32_t>(Value);
    };

#define THEIA_REGISTER_CALLBACK_IMPL(CT, Function, ForWhat)                                                                                                                                                    \
  namespace detail {                                                                                                                                                                                           \
    using CT = theia::detail::CallbackTraits<std::remove_const<decltype(ForWhat)>::type, decltype(Function), ForWhat>;                                                                                         \
  }                                                                                                                                                                                                            \
  template <>                                                                                                                                                                                                  \
  THEIA_EXPORT detail::CT::return_type theia::detail::CrashCallback<detail::CT::code, detail::CT::flags, detail::CT::return_type>::Call(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) { \
    return Function(Code, Param1, Param2, Param3);                                                                                                                                                             \
  }
  } // namespace detail
} // namespace THEIA_REAL_NAMESPACE

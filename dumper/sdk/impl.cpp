#include <Windows.h>

#include <winternl.h>

#include <xutility>

#include <theia_dumper.hpp>

extern "C" IMAGE_DOS_HEADER __ImageBase;

static PVOID s_mapping{};

bool theia_dumper::init(const wchar_t* runtime_name) {
  if (s_mapping)
    return true;
  wchar_t path[512]{};
  auto ret = GetModuleFileNameW((HMODULE)&__ImageBase, path, std::size(path));
  if (!ret)
    return false;
  const auto backslash = wcsrchr(path, L'\\');
  if (!backslash) {
    SetLastError(ERROR_BAD_PATHNAME);
    return false;
  }
  const auto filename = backslash + 1;
  wcscpy_s(filename, std::end(path) - filename, runtime_name);
  const auto file = CreateFileW(
    filename,
    GENERIC_READ | GENERIC_EXECUTE,
    FILE_SHARE_READ,
    nullptr,
    OPEN_EXISTING,
    0,
    nullptr
  );

  bool err = ERROR_SUCCESS;
  if (file != INVALID_HANDLE_VALUE) {
    const auto mapping = CreateFileMappingW(
      file,
      nullptr,
      PAGE_READONLY | SEC_IMAGE,
      0,
      0,
      nullptr
    );
    if (mapping) {
      const auto view = MapViewOfFile(
        mapping,
        FILE_MAP_READ,
        0,
        0,
        0
      );
      if (view) {
        s_mapping = view;
      }
      CloseHandle(mapping);
    } else {
      err = GetLastError();
    }

    CloseHandle(file);
  } else {
    err = GetLastError();
  }

  SetLastError(err);
  return !err;
}

struct theia_dumper_args {
  size_t len;
  decltype(&theia_dumper::make_task) fn;
  void* ntdll;
  void* kernel32;
};

theia_dumper::task* theia_dumper::make_task(task_callback* cb) {
  if (!s_mapping) {
    SetLastError(ERROR_DLL_INIT_FAILED);
    return nullptr;
  }
  theia_dumper_args args{};
  args.len = sizeof(args);
  const auto ntdll = GetModuleHandleW(L"ntdll");
  args.ntdll = ntdll;
  args.kernel32 = GetModuleHandleW(L"kernel32");
  if (!args.ntdll || !args.kernel32) {
    SetLastError(ERROR_DLL_INIT_FAILED);
    return nullptr;
  }
  const auto pRtlNtStatusToDosError = (decltype(&RtlNtStatusToDosError))(GetProcAddress((HMODULE)ntdll, "RtlNtStatusToDosError"));
  if (!pRtlNtStatusToDosError) {
    SetLastError(ERROR_DLL_INIT_FAILED);
    return nullptr;
  }
  const auto dosh = (PIMAGE_DOS_HEADER)s_mapping;
  const auto nth = (PIMAGE_NT_HEADERS)((char*)s_mapping + dosh->e_lfanew);
  const auto entry = (char*)s_mapping + nth->OptionalHeader.AddressOfEntryPoint;
  const auto status = ((NTSTATUS(*)(PVOID, ULONG, PVOID, theia_dumper_args*))entry)(nullptr, 0x5ff21179, nullptr, &args);
  if (!NT_SUCCESS(status)) {
    SetLastError(pRtlNtStatusToDosError(status));
    return nullptr;
  }
  SetLastError(0);
  return args.fn(cb);
}
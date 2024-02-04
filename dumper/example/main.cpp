#include <Windows.h>

#include <ios>
#include <winternl.h>

#include <iomanip>
#include <iostream>
#include <limits>

#include <theia_dumper.hpp>

struct callback final : theia_dumper::task_callback {
  void log(const wchar_t* msg) override {
    std::wcout << msg << std::endl;
  }

  void started() override {
    std::wcout << L"Dumping started.." << std::endl;
  }

  void failed() override {
    std::wcout << L"Dumping failed!" << std::endl;
  }

  void dump_progress(uint32_t progress) override {
    const auto percent = (double)progress / (double)(std::numeric_limits<uint32_t>::max)() * 100.0;
    std::wcout << L"Dump progress: " << std::fixed << std::setprecision(1) << percent << L"%" << std::endl;
  }

  void dump_finished(const wchar_t* path) override {
    std::wcout << L"Dumping finished! Dump: " << path << std::endl;
  }

  void upload_started() override {
    std::wcout << L"Uploading started.." << std::endl;
  }

  void upload_progress(uint32_t progress) override {
    const auto percent = (double)progress / (double)(std::numeric_limits<uint32_t>::max)() * 100.0;
    std::wcout << L"Upload progress: " << std::fixed << std::setprecision(1) << percent << L"%" << std::endl;
  }

  void upload_finished(uint32_t http_code) override {
    std::wcout << L"Uploading finished! (HTTP code " << http_code << L")" << std::endl;
  }

  virtual void success() {
    std::wcout << L"Success!" << std::endl;
  };
};

int main() {
  std::wcout << L"Dumper launched" << std::endl;
  auto res = theia_dumper::init(L"runtime.dll");
  if (res) {
    callback cb{};
    const auto task = theia_dumper::make_task(&cb);
    if (task) {
      auto status = task->initialize();
      if (NT_SUCCESS(status)) {
        task->dump(theia_dumper::dump_flags::none);
        task->close();
      } else {
        std::wcout << L"Task initialization failed with " << std::hex << status << std::endl;
      }
    } else {
      const auto err = GetLastError();
      std::wcout << L"Task creation failed with " << err << std::endl;
    }
  } else {
    const auto err = GetLastError();
    std::wcout << L"Initialization failed with " << err << std::endl;
  }
  std::wcout << L"Press any key to dismiss.." << std::endl;
  getchar();
  return 0;
}

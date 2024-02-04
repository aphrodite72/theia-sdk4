#pragma once
#include <cstdint>

#ifdef STATUS_SUCCESS
#define THEIA_NTSTATUS NTSTATUS
#else
#define THEIA_NTSTATUS long
#endif

namespace theia_dumper {
  enum class dump_flags {
    none,
    full_dump
  };

  struct task_callback {
    virtual ~task_callback() = default;
    virtual void log(const wchar_t* msg) = 0;
    virtual void started() = 0;
    virtual void failed() = 0;
    virtual void dump_progress(uint32_t progress) = 0;
    virtual void dump_finished(const wchar_t* path) = 0;
    virtual void upload_started() = 0;
    virtual void upload_progress(uint32_t progress) = 0;
    virtual void upload_finished(uint32_t http_code) = 0;
    virtual void success() = 0;
  };

  struct task {
    virtual ~task() = default;
    virtual THEIA_NTSTATUS initialize() = 0;
    virtual THEIA_NTSTATUS dump(dump_flags flags) = 0;
    virtual THEIA_NTSTATUS dump_async(dump_flags flags) = 0;
    virtual THEIA_NTSTATUS close() = 0;
  };

  bool init(const wchar_t* runtime_name);
  task* make_task(task_callback* cb);
}

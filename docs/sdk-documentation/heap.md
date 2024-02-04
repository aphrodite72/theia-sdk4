# Protected Heaps

The Theia SDK allows the creation of "protected heaps". These heaps are suitable for allocating small to mid-sized objects whose usage can be captured in [critical sections](https://en.wikipedia.org/wiki/Critical_section). Theia allows full access to the heap and its objects while inside the critical section, but will raise crashes if any access to the heap or its memory is detected while outside of the section. This allows you to isolate important datastructures in a way where any unauthorized attempts at reading them result in a crash of the process.

When outside the critical section, Theia will log all access to objects allocated on the protected heap. This allows Theia to detect memory access originating from:

- The current process, outside of the critical section.
- Any external process attempting to use [ReadProcessMemory](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-readprocessmemory) or similar APIs to read memory.
- Common kernel-mode reading techniques.

While such memory access can be caused by legitimate programs (e.g. anti-virus scanners), they can also originate from cheaters attempting to read internal data structures while the game is outside the critical section. Theia uses a time and frequency based heuristic to predict whether memory access outside the critical section is legitimate or not.

## Basic Usage

### Creating heaps

All Theia protected heaps are created using the `CreateHeap` factory function provided by SDK interface. This function takes two arguments:

- `ReservedSize`: indicating the amount of memory to reserve for the heap, its internal data structures, and the allocated memory
- `MaxAllocSize`: indicating the maximum allowable allocation for this heap

If the current process is not protected (e.g. the Theia SDK is integrated but the executable is not packed), this function will return a dummy heap which directly uses the CRT allocator to serve allocations. This allows you to use protected heaps even when Theia is not applied.

Upon heap creation, Theia will reserve `ReservedSize` bytes of memory. Due to the way virtual memory works, such memory will be shown as in-use by tools such as task manager. Nevertheless, no actual memory will be used until actual content is written to the backing memory. We still recommend to choose a suitable size for the heap, as a bigger heap size may result in larger overhead for the `lock` and `unlock` calls.

There are several additional considerations you should keep in mind when calling this function. We recommend that you review the current Theia SDK documentation for your language (e.g. [C++](./cpp.md)) for a definitive list of valid arguments and other constraints.

### Entering the critical section

Before you can use the resulted heap, your thread must first enter the critical section.

Entering and leaving the critical section is possible in one of two ways:

- Calling `lock` and `unlock` manually.
- Passing the heap to one of C++11 RAII helpers like `std::lock_guard`.

Out of these two, we strongly recommend the use of `std::lock_guard`, as this will ensure that no calls to `unlock` are accidentally forgotten.

Upon locking the heap, Theia will check if any memory access to the heap has occurred since the last time that `unlock` was called. If such access has occurred, and heuristics indicate that the access was likely malicious, Theia will invoke the `ProcessIntegrity::ExternalMemoryAccess` crash callback.

After locking, it is completely legal to access the heap and any objects allocated on it. The `Allocate`, `DeAllocate`, and `ReAllocate` functions exposed by the heap behave similarly to `std::malloc`, `std::free`, and `std::realloc` respectively, barring the restrictions on `ReservedSize` and `MaxAllocSize`. Standard C++ functionality, such as [placement new](https://en.cppreference.com/w/cpp/language/new#Placement_new), can be used to allocate objects within the protected heap.

Once you are done accessing the heap, ensure that you unlock the heap. If using `std::lock_guard`, this will be done automatically. Any further memory access to the heap after the call to `unlock` will be considered as unauthorized and may result in a crash the next time `lock` is called.

The protected heap **is not re-entrant, nor does it contain a synchronization primitive**. Calling `lock` while the heap is already in a critical section, regardless of the thread, will result in undefined behavior and potential deadlocks. If you have multiple threads that need access to a protected heap, you will need to coordinate them using a synchronization primitive (e.g. a mutex) yourself.

### Complete example

The following shows an example program using the [C++ Theia SDK](./cpp.md) to allocate an array of four integers onto the protected heap.

```cpp
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>
#include <theia_sdk.hpp>

THEIA_ONCE();

int main () {
  // ensure we're receiving callbacks
  theia::GetInterface()->ReadyForCallbacks();

  // create protected heap ~64KB in size
  theia::Heap* heap = theia::GetInterface()->CreateHeap(0x10000, 0, nullptr);

  // enter critical section manually
  heap->lock();

  // allocate space for 4 integers
  void* space = heap->Allocate(sizeof(int) * 4);

  // initialize a new value on the heap using placement new (https://en.cppreference.com/w/cpp/language/new#Placement_new)
  int* array = new (space) int[4] { 10, 20, 30, 40};

  // leave critical section manually
  heap->unlock();

  // any access to `array` will be detected on next call to lock, even if it originates in the same
  // process and the same thread
  // printf("%d\n", array[0]); // will be logged and result in a crash if done often enough

  // loop running for extended amount of time
  for (int i = 0; i < 60; ++i) {
    // sleep before entering critical section
    std::this_thread::sleep_for(std::chrono::seconds{1});

    // enter the critical section using std::lock_guard
    std::lock_guard<theia::Heap> lock_guard(*heap);

    // any access to the heap and its memory in the remainder of this scope is legal
    array[0] += 10;
    printf("array[0] = %d\n", array[0]);

    // RAII helper automatically leaves critical section
  }

  // deallocation also requires that you are in the critical section
  {
    std::lock_guard<theia::Heap> lock_guard(*heap);
    heap->DeAllocate(array);
  }

  // destroy the heap, freeing the reserved memory. Can be done outside the
  // critical section
  heap->Destroy();

  // note that you **MUST NOT** be in a std::lock_guard guarded scope when calling
  // destroy, as it is illegal to access `heap` after destroying it. To illustrate:
  {
    // DON'T DO THIS!
    std::lock_guard<theia::Heap> lock_guard(*heap);
    heap->Destroy();
    // UB: std::lock_guard::~lock_guard gets called here, which calls heap->unlock
    // this is undefined behavior: the backing memory for `heap` is already freed
  }
}
```

## Limitations

While the protected heap is a powerful primitive for preventing unauthorized access to important data structures, there are a few considerations and limitations that you must be aware of before integrating it in your application:

- The sections of your application that need to access the protected memory must be capturable in a well-defined critical section for the heap to be useful.
- The base protected heap only aligns pointers by 16 bytes. Any bigger alignments require manual alignment.
- The maximum capacity of the heap must be specified upfront, and cannot be adjusted later.
- The heap is intended for use with small to medium-sized objects. Very large allocation requests (those exceeding roughly 1024kB in size) may fail.

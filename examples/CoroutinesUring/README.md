# C++ Coroutines: Use case for Kernel io_uring development

*May 19^th, 2025 By Zeeshan Qazi*

Co-routines in C++ are a useful tool for writing easy to read sequential code, without having to write complex state machines, particularly for use cases where readability of the control flow is important. One of these use cases is asynchronous I/O handling in C++ applications. This is where the io_uring functionality provided by Linux could be used effectively.

Co-routines appear to be difficult to use at a first glance, and seem quite overwhelming compared to similar concepts found in other languages. But this apparent complexity actually allows one to provide greater customization opportunities to meet one's usecase. Typically, coroutines in C++ may rely on heap for storage of the coroutines' frames, but richness of `promise_type` allows for mechanism for even overriding this default behavior.

Before we start writing code for the co-routines, we need to understand some basic concepts:

 

- `task` class – This class is typically used to give details about the co-routine implementation, as well as a container for co-routine internals, such as definition of `promise_type`, and also storage of the handle to the co-routine.
- `task::promise_type` type – This type describes the co-routine's default behaviors, such as initial or final suspensions, dealing with void returns, value returns (if supported), and handling unhandled exceptions in the co-routing contexts. This type may optionally provide `**operator new**` and `**operator delete**` to provide custom allocators.
- `awaiter` type –An instance of this type is used with `**co_await**` keyword to trigger suspension of a co-routine in a method that returns `task` class. This type provides mechanisms for checking co-routine readiness (`await_ready()` method), suspending co-routine (`await_suspend(std::coroutine_handle&lt;&gt;)` method), and resuming co-routine (`await_resume()` method).

You may read further details about the implementation of boil-plate code and various types of co-routines at [C++ Reference: Coroutines(C++20)](https://en.cppreference.com/w/cpp/language/coroutines).

In our article we are providing a simple HTTP server, which basically provides a simple example of usage of [liburing](https://github.com/axboe/liburing).This library is now part of many Linux distributions and utilize the system call interface provided by new linux kernels for uring. The idea behind usage of this library is to minimize the system calls, provide ability for sharing / gifting pages to/from user code to the kernel, and other such facilities to minimize latency. Though the library and the underlying feature is fairly useful, there are a few caveats to its usage, as there are lots of [vulnerabilities](https://cve.mitre.org/cgi-bin/cvekey.cgi?keyword=io_uring), making its usage unsafe in untrusted contexts.

Our example essentially demonstrates two things:

- How to implement io_uring operation queuing and completion events processing using co-routines?
- How to override the allocator for the co-routines? 

Our example is not:

- a demonstration of a performant implementation, as it still uses classes such as `std::string` and `std::stringstream`.
- a foolproof implementation designed to prevent exploitation of vulnerabilities in io_uring interface, its usage, or the implemented web server. Though we do provide some protection against access of resources outside the base directory by using a deliberate URI or by following a symbolic link that leads to the same.

Let us take a look at our implementation to see how the co-routines can be used together with liburing by walking through various classes and files:

- `async_io_uring_task_allocator` class – This class provides a pool style allocator for co-routine frames. The allocator basically pre-mallocs a number of frames in a pool keyed by co-routine frame's size. Allocations are attempted to be fulfilled from this pool first, and then we fallback to the regular malloc.
- `async_io_uring_task` class – This class is used as a return value for all the co-routines that are managed by our `io_uring_context` class. By managed, we mean dealing with the suspensions and resumptions of these co-routines.
- `asyc_io_uring_task::promise_type` class – This class essentially gives details about co-routines declared with `async_io_uring_task` class as return type:   `get_return_object()` method – provides a mechanism of getting a task from the promise.
- `initial_suspend()` method – tells co-routine mechanism that our co-routines do not start up suspended.
- `final_suspend()` method – tells co-routine mechanism that our co-routines suspend when they are finished.
- `**operator new(...)**` – demonstrates how to associate a custom allocator with the co-routines of this type. Notice the signature of the operator contains the allocator type, and this allocator type is hence required to be the first argument of each co-routine that is typed for our task. As you can see all the arguments from the co-routine being invoked are potentially visible from the new operator, even though we only care about the first argument, which in our case is the allocator.
- `**operator delete(...)**` – demonstrates how a deleter is written to use a custom allocator. As you can see, the allocator has to be stashed after the actual object in the allocation, as the information about the allocator or the co-routine arguments is not available from this method's call-site.

- `async_io_uring_tasks_list` type-alias – is used for storing the tasks that are still in progress to avoid their resources from being released.
- `io_uring_context` class – initializes and stores the actual I/O uring, and provides an event dispatcher loop for resumption of co-routines on completion of the queued I/O operations. It also provides a backup storage for when the ring is saturated to store further suspension so they may be recovered when space becomes available in the uring.
- `io_uring_awaitable` class – is used as a return type by asynchronous operations in our example to provide a mechanism for setting up `**io_uring_sqe**` instances, which are essentially I/O operations to the kernel. It is like a typical awaitable described above and in co-routine documentation, with additional ability to use a secondary queue for when the ring itself is saturated.
- `async_file_descriptor` class and related helper methods – provide a mechanism for operations to I/O uring, while suspending the calling task.
- `io_uring_context::dispatch_events()` method – is used for doing actual submission of I/O operations, and then waiting for completion of those operations and then dispatching them by resuming the coroutines associated with those operations. One thing to note is that we are using more inefficient but easier to understand `io_uring_wait_cqe(...)` function, which only gives us one completed CQE at a time for ease of reading, but other options exist such as `io_uring_peek_batch_cqe`.
- `simple_http_server.cpp` file – is the main module where all the facilities implemented in this example are demonstrated.

## Pre-requisites

- Boost.LockFree library is used in our allocator to provide a freelist.
- liburing library is used to access the kernel's uring interface without dealing with system calls directly.

[   io_uring_coroutine_helpers.hpp](javascript:DoLink('/download-sourcecode.php?Example=io_uring_coroutine_helpers');) 
[   io_uring_coroutine_helpers.cpp](javascript:DoLink('/download-sourcecode.php?Example=io_uring_coroutine_helpers.cpp');) 
[   async_io_uring_task.hpp](javascript:DoLink('/download-sourcecode.php?Example=async_io_uring_task');) 
[   simple_http_server.cpp](javascript:DoLink('/download-sourcecode.php?Example=simple_http_server');) 

- **G++ Compilation Instructions:** `g++ -DDEBUG_ASYNC_IO_URING_TASK_ALLOCATOR --std=c++23 *.cpp -O0 -g3 -fcoroutines -luring -o simple_http_server`

---

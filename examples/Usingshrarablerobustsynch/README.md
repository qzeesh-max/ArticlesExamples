# Using Robust Shareable Synchronization Primitives

*February 23^rd, 2013 By Zeeshan Qazi*

Shared memory is a very fast mechanism for inter-process communication, but this speed comes at a price. Applications that share memory and use this memory to queue and dequeue data between each other or for simply sharing memory mapped data structures, expose themselves to the possibility of one of the applications participating in this sharing crashing while holding ownership of a synchronization primitive or worse crashing without completing the operation which was meant to be protected by a synchronization primitive. The former could leave the set of applications that were participating in sharing memory to end up with livelocks thereby needing those livelocks to be detected and broken, and the latter could leave the application data to be in an inconsistent state leading to a crash or worse.

The `boost::interprocess` namespace contains many cross platform as well as platform specific interprocess communication mechanisms. This namespace is particularly helpful for people wishing to write interprocess applications that are cross platform. The namespace provides many synchronization primitives that may be used for synchronizing operations or signalling events between multiple processes while having these structures either embedded in a shared memory region or be named globally so any application may refer to them by name. The scope of this article is to deal with the short coming of the synchronization primitives provided by `boost::interprocess` that are meant to be embedded in shared memory regions. We will deal with the short coming that exists in `interprocess_mutex`, `interprocess_recursive_mutex` and `interprocess_condition` that has to do with being able to deal with a thread dying while holding a mutex by implementing robust mutexes that automatically get released upon a thread dying.

The fact is that most modern operating systems that implement POSIX or even Windows platform (including Wine and ReactOS), do indeed support a mechanism for detecting and handling thread dying while holding a mutex. The problem lies in the implementation of shared memory objects that is provided by `boost::interprocess` as the mutexes used by the library are not used in a manner to be able to detect thread death while owning one of the mutexes, therefore using the default `boost::interprocess` objects for shared memory leaves you at risk of livelocks upon such application failures. But a great thing about `boost` and `STL` is that their usage of templates provides you the ability to substitute default functionality with alternative functionality.

Now let us examine how we are going to implement our solution to this problem. We would be implementing this solution for the Windows API as well as most modern POSIX implementations. Since there is a massive divergence between the two of them, our implementation would have different code running on both platforms. We would now dive into the implementations of the mutexes provided by these operating systems that support this functionality automatically along with the problems of using these implementations in shared memory regions.

### Windows

Windows has always provided ability of having a Mutex released when a thread dies while holding it. The only problem with the Windows Mutexes is that for purpose of sharing them between multiple processes they must be named. Due to the fact race conditions can occur when a Mutex seizes to exist if its handle were to be shared and duplicaed directly through another means every process is required to get a handle to the mutex via a call to `CreateMutex` or `OpenMutex` with the name of the Mutex. Since Windows Mutexes are handle based those handles cannot be placed inside the shared memory regions directly as they are reference counted against their ownership by individual processes. This opens up a new issue of having to store in the shared memory region some reference such as the name of the mutex, while having the handle to the mutex stored within the application itself. This brings up the issue of having more complex data represented outside the shared memory while having a reference to it stored safely inside the memory region so any process in need of the same object may create it outside the memory region. We shall look at a class shortly that will provide us with the ability to have references to the objects stored in the shared memory while having a full fledged object created outside the shared memory region with the intent of using such a class to hold in individual application a named mutex handle as needed.

### POSIX-based Operating Systems

POSIX since [POSIX.1-2008 issue 7](https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xsh_chap01.html) supports robust mutexes. This functionality was implemented soon after in most of the Operating Systems supporting POSIX that are still compliant with the standards. The good thing about the POSIX mutexes is that they can be created with attributes that allow them to be embedded directly inside shared memory regions, there by eliminating the need for having other mechanisms for holding objects outside the shared memory region.The robust mutex functionality is basically implemented using system calls by the POSIX threads library with kernel functionality provided by enhancements to the `futex` interfaces. A glimpse of how this implementation works can be seen in the man pages for `get_robust_list` and `set_robust_list` system calls.

For our implementation, we would not need to do anything even half as complicated as dealing directly with the `futexes` or with the per-thread `robust_list`. We would instead let the POSIX threads setup the necessary `robust_list` automatically by telling the POSIX mutexes at initialization by providing them `pthread_mutexattr_t` with the `pshared` and `robust` attribute set to `PTHREAD_PROCESS_SHARED` and `PTHREAD_MUTEX_ROBUST` respectively. These two changes to the attributes of `pthread_mutex_t` are sufficient for providing us the robust mutex functionality on Linux and other POSIX implementations. Setting these attributes has an impact on the cost of `pthread_mutex_t` objects as more overhead is needed for providing this functionality, therefore unlike Windows on POSIX this additional functionality needs to be enabled using the mutex attributes.

### Implementation

Now let us look at our implementation of the robust shareable synchronization primitives for Windows and POSIX. We will first start with the restriction that Windows Mutex handles cannot be stored inside a shared memory region because of several reasons such as their lifetime depending on reference counts and sharability restrictions on the handles across processes. We would go around this issue by implementing proxy references capable of providing ability to have objects outside the shared memory region referenced from within them, while not compromising the ability to resize `boost::interprocess`' `segment_manager` for `managed_shared_memory` objects.

Let us look at the implementation of `shareable_object_slot`, which we are implementing as cross platform using `boost` as necessary despite no such need to do so for this article. The implementation of `shareable_object_slot` for POSIX relies on `libuuid` to generate UUID's.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=shareable_object_slot');)

*Refer to the source file `shareable_object_slot.hpp` in this directory.*

For our implementation we would need three classes to be defined that will provide us with necessary functionality for robust mutexes and auto reset events:

- `robust_shareable_mutex` – This class provides a non-recursive mutex on POSIX platforms, but due to limitations on Windows it provides a recursive mutex on Windows. But this is fine because the additional functionality does not break anything.

- `robust_shareable_recursivemutex` – This class provides a recursive mutex that is shareable inside shared memory on both POSIX and Windows.

- `robust_shareable_autoresetevent` – This class provides an Auto Reset Event implementation similar to Windows implementation except it is shareable inside shared memory.

Let us look at the implementation of `robust_shareable_mutex`. You will notice that this implementations is fairly trivial.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=robust_shareable_mutex');)

*Refer to the source file `robust_shareable_mutex.hpp` in this directory.*

Now let us look at the implementation of `robust_shareable_recursivemutex`. You will notice that compared to the previous implementation there is only one minor difference where we apply the `PTHREAD_MUTEX_RECURSIVE` attribute to the mutex object at creation.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=robust_shareable_recursivemutex');)

*Refer to the source file `robust_recursive_shareable_mutex.hpp` in this directory.*

The two classes that we just created are sufficient for replacing the `mutex_family` template parameter that is required for creation of `MemoryAlgorithm` concept based template class used by `managed_shared_memory` objects, but to support our example we need to implement yet another class that provides us Auto Reset Events across both platforms.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=robust_shareable_autoresetevent');)

*Refer to the source file `robust_sharable_auto_reset.hpp` in this directory.*

Now let us look at the example program that utilizes the above classes to demonstrate our implementation of the primitives. The example below is a single program that acts as a listener or a poster depending on the paremeter provided. The **server** option initiates a listener, and you may initiate as many of them as you like simultaenously, whereas the **client** option initiates a poster and you may  initiate as many of them as you like simultaneously. The example has a call to `exit` function to terminate the program while holding a mutex after posting 5000 messages, but you may change that to any random number or location in the example, or break the client early most of the time without corrupting the segment manager. You can run the client in a loop using shell scripts to test this code.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=robust_shareable_primitives');)
   Compilation Instructions: 
- **Linux:**

*Refer to the source file `robust_shareable_primitives.hpp` in this directory.*

I hope this article has been helpful to those who have been seeking a solution to a common problem while dealing with shared memory. Other ways around the issues caused by synchronizing objects inside the shared memory can also involve use of RCUs in shared memory.

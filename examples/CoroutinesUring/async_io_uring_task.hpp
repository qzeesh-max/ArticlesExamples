#ifndef ASYNC_IO_URING_TASK_H
#define ASYNC_IO_URING_TASK_H

#include <boost/lockfree/queue.hpp>
#include <coroutine>
#include <exception>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>

#ifdef DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR
#include <iostream>
#endif //! defined(DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR)

struct async_io_uring_task_allocator {
  size_t pool_size;
  std::shared_mutex pool_head_mutex;

  struct pool_head {
    size_t adjusted_object_size;
    boost::lockfree::queue<void *> free_list;
    char *allocated_pool;
    char *pool_end;

    pool_head(async_io_uring_task_allocator *allocator, size_t pool_size,
              size_t object_size)
        : adjusted_object_size(object_size +
                               (object_size - object_size % sizeof(size_t[2]))),
          free_list(pool_size), allocated_pool(reinterpret_cast<char *>(
                                    malloc(pool_size * adjusted_object_size))),
          pool_end(allocated_pool + pool_size * adjusted_object_size) {
      for (auto *entry = allocated_pool; entry != pool_end;
           entry += adjusted_object_size) {
        *reinterpret_cast<async_io_uring_task_allocator **>(
            entry + object_size) = allocator;
        free_list.push(entry);
      }
#ifdef DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR
      std::cout << "New allocator allocated for object sized " << object_size
                << " and pool of size " << pool_size << std::endl;
#endif //! defined(DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR)
    }

    bool push(void *&entry) { return free_list.push(entry); }

    bool pop(void *&entry) { return free_list.pop(entry); }

    bool is_in_pool(void *entry) const {
      char *c = reinterpret_cast<char *>(entry);

      return (c >= allocated_pool) && (c < pool_end);
    }

    ~pool_head() { free(allocated_pool); }
  };

  std::unordered_map<size_t, pool_head> pool_heads;

  void *allocate(size_t size) {
    auto *p = find_or_alloc_block(size);

    if (p != nullptr)
      return p;

    p = alloc_pool_and_get_block(size);

    return p;
  }

  void deallocate(void *p, size_t size) {
    {
      std::shared_lock lock(pool_head_mutex);

      if (auto it = pool_heads.find(size); it != pool_heads.end()) {
        if (it->second.is_in_pool(p)) {
          it->second.push(p);
          return;
        }
      }
    }

    ::operator delete(p);
  }

private:
  void *find_or_alloc_block(size_t size) {
    {
      std::shared_lock lock(pool_head_mutex);

      if (auto it = pool_heads.find(size); it != pool_heads.end()) {
        void *result;
        if (it->second.pop(result))
          return result;
      } else {
        // parent must allocate a pool
        return nullptr;
      }
    }

    return regular_alloc(size);
  }

  void *alloc_pool_and_get_block(size_t size) {
    std::unique_lock lock(pool_head_mutex);

    auto [it, _] =
        pool_heads.emplace(std::piecewise_construct, std::make_tuple(size),
                           std::make_tuple(this, pool_size, size));

    void *result;

    if (it->second.pop(result))
      return result;

    lock.unlock();

    return regular_alloc(size);
  }

  void *regular_alloc(size_t size) {
    // fall-back: allocate extra-room to record the allocator, and then return
    // the pointer
    char *p = reinterpret_cast<char *>(
        ::operator new(size + (size - size % sizeof(size_t[2]))));

    // stash the allocator there
    *reinterpret_cast<async_io_uring_task_allocator **>(p + size) = this;

    return p;
  }
};

struct async_io_uring_task {
  struct promise_type;

  using handle_type = std::coroutine_handle<promise_type>;

  handle_type coro_handle;

  async_io_uring_task(handle_type h) : coro_handle(h) {}

  async_io_uring_task(async_io_uring_task &&t) noexcept
      : coro_handle(std::exchange(t.coro_handle, nullptr)) {}

  ~async_io_uring_task() {
    if (coro_handle) {
      coro_handle.destroy();
    }
  }

  bool resume() {
    if (!coro_handle || coro_handle.done()) {
      return false;
    }

    coro_handle.resume();

    return !coro_handle.done();
  }

  bool done() const { return coro_handle.done(); }

  struct promise_type {
    async_io_uring_task get_return_object() {
      return {handle_type::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept { return {}; }

    std::suspend_always final_suspend() noexcept { return {}; }

    void return_void() {}

    void unhandled_exception() { std::terminate(); }

    template <typename... Arguments>
    inline static void *operator new(size_t size,
                                     async_io_uring_task_allocator &allocator,
                                     Arguments &&...) {
#ifdef DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR
      std::cout << "Allocating coroutine with size " << size << std::endl;
#endif //! defined(DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR)

      return allocator.allocate(size);
    }

    static void operator delete(void *p, size_t size) {
      auto *allocator = *reinterpret_cast<async_io_uring_task_allocator **>(
          reinterpret_cast<char *>(p) + size);

#ifdef DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR
      std::cout << "Delocating coroutine with size " << size << std::endl;
#endif //! defined(DEBUG_ASYNC_IO_URING_TASK_ALLOCATOR)

      allocator->deallocate(p, size);
    }
  };
};

using async_io_uring_tasks_list = std::list<async_io_uring_task>;

#endif // !defined(ASYNC_IO_URING_TASK_H)

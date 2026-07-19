#ifndef IOURING_EXAMPLE_COROUTINEHELPERS_H
#define IOURING_EXAMPLE_COROUTINEHELPERS_H

#include <coroutine>
#include <deque>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <liburing.h>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct io_uring_awaitable;

struct io_uring_context {
  struct io_uring ring;
  unsigned queue_depth;
  std::deque<io_uring_awaitable *> pending_awaitable;
  bool terminated = false;

  io_uring_context(unsigned queue_depth = 1024) : queue_depth(queue_depth) {
    if (io_uring_queue_init(queue_depth, &ring, 0) < 0) {
      throw std::runtime_error("Failed to initialize io_uring");
    }
  }

  ~io_uring_context() { io_uring_queue_exit(&ring); }

  struct io_uring *get_ring() {
    return &ring;
  }

  void terminate() { terminated = true; }

  void dispatch_events();
};

struct io_uring_awaitable {
  io_uring_context &ring_context;
  std::function<void(struct io_uring_sqe *)> prep_sqe_func;
  std::coroutine_handle<> awaiting_coroutine;
  int cqe_res;

  io_uring_awaitable(io_uring_context &r,
                     std::function<void(struct io_uring_sqe *)> prep)
      : ring_context(r), prep_sqe_func(std::move(prep)) {}

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) noexcept {
    awaiting_coroutine = h;
    auto *ring_ptr = ring_context.get_ring();
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring_ptr);

    if (sqe == nullptr) {
      ring_context.pending_awaitable.push_back(this);
      return;
    }

    queue_to_sqe(sqe);
  }

  void queue_to_sqe(struct io_uring_sqe *sqe) {
    prep_sqe_func(sqe);
    io_uring_sqe_set_data(sqe, this);
  }

  int await_resume() noexcept {
    // cqe_res should be populated by the main loop before resuming
    return cqe_res;
  }
};

class async_file_descriptor {
  io_uring_context &ring_context;
  int fd;

public:
  async_file_descriptor(io_uring_context &ring_context, int fd)
      : ring_context(ring_context), fd(fd) {}

  io_uring_awaitable async_accept(struct sockaddr *addr, socklen_t *addrlen,
                                  int flags) {
    return io_uring_awaitable(
        ring_context, [=, this](struct io_uring_sqe *sqe) {
          io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
        });
  }

  io_uring_awaitable async_recv(void *buffer, size_t bufferlen, int flags) {
    return io_uring_awaitable(
        ring_context, [=, this](struct io_uring_sqe *sqe) {
          io_uring_prep_recv(sqe, fd, buffer, bufferlen, flags);
        });
  }

  io_uring_awaitable async_send(const void *buffer, size_t bufferlen,
                                int flags) {
    return io_uring_awaitable(
        ring_context, [=, this](struct io_uring_sqe *sqe) {
          io_uring_prep_send(sqe, fd, buffer, bufferlen, flags);
        });
  }

  io_uring_awaitable async_pread(void *buf, size_t count, off_t offset) {
    return io_uring_awaitable(ring_context,
                              [=, this](struct io_uring_sqe *sqe) {
                                io_uring_prep_read(sqe, fd, buf, count, offset);
                              });
  }

  io_uring_awaitable async_pwrite(const void *buf, size_t count, off_t offset) {
    return io_uring_awaitable(
        ring_context, [=, this](struct io_uring_sqe *sqe) {
          io_uring_prep_write(sqe, fd, buf, count, offset);
        });
  }

  io_uring_awaitable async_shutdown(int how) {
    return io_uring_awaitable(ring_context,
                              [=, this](struct io_uring_sqe *sqe) {
                                io_uring_prep_shutdown(sqe, fd, how);
                              });
  }

  io_uring_awaitable async_close() {
    return io_uring_awaitable(
        ring_context,
        [=, this](struct io_uring_sqe *sqe) { io_uring_prep_close(sqe, fd); });
  }

  auto handle() const { return fd; }
};

inline io_uring_awaitable async_open(io_uring_context &ring_context,
                                     const char *path, int flags, mode_t mode) {
  return io_uring_awaitable(ring_context, [=](struct io_uring_sqe *sqe) {
    io_uring_prep_openat(sqe, AT_FDCWD, path, flags, mode);
  });
}

inline io_uring_awaitable async_statx(io_uring_context &ring_context,
                                      const char *pathname, int flags,
                                      unsigned int mask,
                                      struct statx *statxbuf) {
  return io_uring_awaitable(ring_context, [=](struct io_uring_sqe *sqe) {
    io_uring_prep_statx(sqe, AT_FDCWD, pathname, flags, mask, statxbuf);
  });
}

#endif //! defined(IOURING_EXAMPLE_COROUTINEHELPERS_H)

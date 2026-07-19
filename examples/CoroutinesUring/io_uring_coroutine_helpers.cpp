#include "io_uring_coroutine_helpers.hpp"

void io_uring_context::dispatch_events() {
  while (!terminated) {
    // Submit all SQEs that were prepared by co_await
    // In a more complex app, you might only submit if there's work.
    int ret = io_uring_submit(&ring);

    if (ret < 0) {
      std::cerr << "io_uring_submit error: " << strerror(-ret) << std::endl;
      // Potentially break or handle specific errors
      if (ret == -EBUSY || ret == -EAGAIN) {
        // These might be recoverable, retry or continue
      } else {
        break; // More severe error
      }
    }

    struct io_uring_cqe *cqe;

    // Wait for one completion. In a real server, you might want
    // io_uring_peek_batch_cqe or a loop with io_uring_wait_cqe_nr /
    // io_uring_peek_cqe
    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
      if (ret == -EINTR) { // Interrupted by signal, continue
        continue;
      }
      std::cerr << "io_uring_wait_cqe error: " << strerror(-ret) << std::endl;
      break;
    }

    void *data = io_uring_cqe_get_data(cqe);
    // This assumes data is the address of the coroutine_handle
    auto *actual_awaiter = reinterpret_cast<io_uring_awaitable *>(data);
    if (actual_awaiter) { // Check if data is not null (it shouldn't be if
                          // set)
      actual_awaiter->cqe_res = cqe->res; // Set the result in the awaiter
      if (actual_awaiter->awaiting_coroutine) {
        actual_awaiter->awaiting_coroutine
            .resume(); // Resume the stored coroutine
      } else {
        std::cerr << "Error: CQE user_data did not point to a valid "
                     "awaiter or coroutine handle."
                  << std::endl;
      }
    } else {
      std::cerr << "Error: CQE user_data was null." << std::endl;
    }

    io_uring_cqe_seen(&ring, cqe);

    // if we ever run into a scenario that we had too many waiting tasks,
    // we will use a second queue, that will be used to queue tasks when
    // completions happen.
    while (!pending_awaitable.empty()) {
      auto *actual_awaiter = pending_awaitable.front();
      struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

      if (sqe == nullptr)
        break;

      actual_awaiter->queue_to_sqe(sqe);
      pending_awaitable.pop_front();
    }
  }
}

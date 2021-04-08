/*
 * Copyright (c) 2019, 2020, 2021 SiKol Ltd.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <linux/io_uring.h>

#include <liburing.h>

#include <cassert>
#include <system_error>

#include <sk/async_invoke.hxx>
#include <sk/posix/detail/io_uring_reactor.hxx>
#include <sk/posix/error.hxx>
#include <sk/reactor.hxx>

namespace sk::posix::detail {

    struct co_sqe_wait final {
        co_sqe_wait(io_uring_reactor *reactor_, io_uring_sqe *sqe_)
            : reactor(reactor_), sqe(sqe_)
        {
            io_uring_sqe_set_data(sqe, this);
        }

        io_uring_reactor *reactor;
        io_uring_sqe *sqe;
        std::int32_t ret = -1;
        std::uint32_t flags = 0;
        coroutine_handle<> coro_handle;
        std::mutex mutex;

        bool await_ready()
        {
            return false;
        }

        bool await_suspend(coroutine_handle<> coro_handle_)
        {
            coro_handle = coro_handle_;
            std::lock_guard lock(mutex);
            reactor->_put_sq(sqe);
            return true;
        }

        int await_resume()
        {
            // Don't allow the wait object to be destroyed until the reactor
            // has released the lock.
            std::lock_guard lock(mutex);
            return ret;
        }
    };

    auto io_uring_reactor::make(workq *q) -> std::unique_ptr<io_uring_reactor>
    {
        // Create the ring.
        auto reactor_ = new io_uring_reactor(q);
        auto reactor = std::unique_ptr<io_uring_reactor>(reactor_);

        auto ret = io_uring_queue_init(
            _max_queue_size, &reactor->ring, IORING_SETUP_CLAMP);
        if (ret < 0)
            return nullptr;

        // Check the ring supports the features we need.
        unsigned required_features = IORING_FEAT_NODROP | IORING_FEAT_RW_CUR_POS;
        if ((reactor->ring.features & required_features) != required_features)
            return nullptr;

        auto probe = io_uring_get_probe_ring(&reactor->ring);
        if (!probe)
            return nullptr;
        std::unique_ptr<io_uring_probe, void (*)(io_uring_probe *)> probe_(
            probe, io_uring_free_probe);

        // Check the ring supports the opcodes we need.
        if (!io_uring_opcode_supported(probe, IORING_OP_NOP))
            return nullptr;
        if (!io_uring_opcode_supported(probe, IORING_OP_OPENAT))
            return nullptr;
        if (!io_uring_opcode_supported(probe, IORING_OP_CLOSE))
            return nullptr;
        if (!io_uring_opcode_supported(probe, IORING_OP_READ))
            return nullptr;
        if (!io_uring_opcode_supported(probe, IORING_OP_WRITE))
            return nullptr;

        // Reactor is okay.
        return reactor;
    }

    // Caller must call io_uring_submit().
    auto io_uring_reactor::_try_put_sq(io_uring_sqe *newsqe) -> bool
    {
        auto sqe = io_uring_get_sqe(&ring);
        if (!sqe)
            return false;

        std::memcpy(sqe, newsqe, sizeof(*sqe));
        return true;
    }

    auto io_uring_reactor::_put_sq(io_uring_sqe *newsqe) -> void
    {
        // Lock here to avoid a race between multiple threads calling
        // io_uring_submit().
        std::lock_guard _(_sq_mutex);

        // std::cerr << "trying to queue\n";
        if (_try_put_sq(newsqe)) {
            auto r = io_uring_submit(&ring);
            if (r >= 0 || r == -EBUSY)
                return;
            else
                abort();
        } else
            _pending.push_back(newsqe);
    }

    io_uring_reactor::io_uring_reactor(workq *q) : _workq(*q) {}

    auto io_uring_reactor::io_uring_thread_fn() -> void
    {
        io_uring_cqe *cqe{};

        for (;;) {
            int did_requests = 0;

            // Process any pending CQEs.

            auto r = io_uring_wait_cqe(&ring, &cqe);
            assert(r == 0);

            do {
                // user_data == null means a shutdown request.
                if (cqe->user_data == 0)
                    return;

                // Resume the queued coro.
                auto *cstate = reinterpret_cast<co_sqe_wait *>(cqe->user_data);
                std::lock_guard h_lock(cstate->mutex);
                cstate->ret = cqe->res;
                cstate->flags = cqe->flags;
                _workq.post([&handle=cstate->coro_handle] { handle.resume(); });

                io_uring_cqe_seen(&ring, cqe);
                ++did_requests;
            } while (io_uring_peek_cqe(&ring, &cqe) == 0);

            {
                // Queue any pending SQEs.
                std::lock_guard sq_lock(_sq_mutex);

                while (!_pending.empty()) {
                    if (_try_put_sq(_pending.front()))
                        _pending.pop_front();
                    else
                        break;
                }

                io_uring_submit(&ring);
            }
        }
    }

    auto io_uring_reactor::start() -> void
    {
        io_uring_thread =
            std::thread(&io_uring_reactor::io_uring_thread_fn, this);
        _workq.start_threads();
    }

    auto io_uring_reactor::stop() -> void
    {
        std::cerr << "stopping reactor\n";

        io_uring_sqe shutdown_sqe{};
        io_uring_prep_nop(&shutdown_sqe);
        shutdown_sqe.fd = -1;
        _put_sq(&shutdown_sqe);

        _workq.stop();
        io_uring_thread.join();
    }

    auto io_uring_reactor::post(std::function<void()> fn) -> void
    {
        _workq.post(std::move(fn));
    }

    /*************************************************************************
     *
     * POSIX async API.
     *
     */

    auto io_uring_reactor::async_fd_open(char const *path, int flags, int mode)
        -> task<expected<int, std::error_code>>
    {
        io_uring_sqe sqe{};
        io_uring_prep_openat(&sqe, AT_FDCWD, path, flags, mode);
        co_return co_await co_sqe_wait(this, &sqe);
    }

    auto io_uring_reactor::async_fd_close(int fd)
        -> task<expected<int, std::error_code>>
    {
        io_uring_sqe sqe{};
        io_uring_prep_close(&sqe, fd);
        co_return co_await co_sqe_wait(this, &sqe);
    }

    auto io_uring_reactor::async_fd_read(int fd, void *buf, std::size_t n)
        -> task<expected<ssize_t, std::error_code>>
    {
        io_uring_sqe sqe{};
        io_uring_prep_read(&sqe, fd, buf, n, -1);
        co_return co_await co_sqe_wait(this, &sqe);
    }

    auto io_uring_reactor::async_fd_pread(int fd,
                                          void *buf,
                                          std::size_t n,
                                          off_t offs)
        -> task<expected<ssize_t, std::error_code>>
    {
        io_uring_sqe sqe{};
        io_uring_prep_read(&sqe, fd, buf, n, offs);
        co_return co_await co_sqe_wait(this, &sqe);
    }

    auto
    io_uring_reactor::async_fd_write(int fd, void const *buf, std::size_t n)
        -> task<expected<ssize_t, std::error_code>>
    {
        io_uring_sqe sqe{};
        io_uring_prep_write(&sqe, fd, buf, n, -1);
        co_return co_await co_sqe_wait(this, &sqe);
    }

    auto io_uring_reactor::async_fd_pwrite(int fd,
                                           void const *buf,
                                           std::size_t n,
                                           off_t offs)
        -> task<expected<ssize_t, std::error_code>>
    {
        io_uring_sqe sqe{};
        io_uring_prep_write(&sqe, fd, buf, n, offs);
        co_return co_await co_sqe_wait(this, &sqe);
    }

} // namespace sk::posix::detail

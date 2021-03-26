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

#ifndef SK_CIO_POSIX_IO_URING_REACTOR_HXX_INCLUDED
#define SK_CIO_POSIX_IO_URING_REACTOR_HXX_INCLUDED

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <linux/io_uring.h>

#include <atomic>
#include <coroutine>
#include <cstring>
#include <deque>
#include <iostream>
#include <system_error>
#include <thread>

#include <liburing.h>

#include <sk/cio/concepts.hxx>
#include <sk/cio/posix/fd.hxx>
#include <sk/cio/task.hxx>
#include <sk/cio/workq.hxx>

namespace sk::cio::posix {

    struct io_uring_reactor final {

        // Try to create a new io_uring_reactor; returns NULL if we can't
        // use io_uring on this system.
        static auto make(workq &) -> std::unique_ptr<io_uring_reactor>;

        io_uring_reactor(io_uring_reactor const &) = delete;
        io_uring_reactor &operator=(io_uring_reactor const &) = delete;
        io_uring_reactor(io_uring_reactor &&) noexcept = delete;
        io_uring_reactor &operator=(io_uring_reactor &&) noexcept = delete;
        ~io_uring_reactor() = default;

        // Start this reactor.
        auto start() -> void;

        // Stop this reactor.
        auto stop() -> void;

        // Post work to the reactor's thread pool.
        auto post(std::function<void()> fn) -> void;

        // POSIX async API
        [[nodiscard]] auto async_fd_open(char const *path, int flags, int mode)
            -> task<expected<int, std::error_code>>;

        [[nodiscard]] auto async_fd_close(int fd)
            -> task<expected<int, std::error_code>>;

        [[nodiscard]] auto async_fd_read(int fd, void *buf, std::size_t n)
            -> task<expected<ssize_t, std::error_code>>;

        [[nodiscard]] auto
        async_fd_pread(int fd, void *buf, std::size_t n, off_t offs)
            -> task<expected<ssize_t, std::error_code>>;

        [[nodiscard]] auto
        async_fd_write(int fd, void const *buf, std::size_t n)
            -> task<expected<ssize_t, std::error_code>>;

        [[nodiscard]] auto
        async_fd_pwrite(int fd, void const *buf, std::size_t n, off_t offs)
            -> task<expected<ssize_t, std::error_code>>;

        // Possibly shouldn't be public.
        auto _put_sq(io_uring_sqe *sqe) -> void;

    private:
        explicit io_uring_reactor(workq&);

        std::mutex _sq_mutex;

        void io_uring_thread_fn();
        std::thread io_uring_thread;

        workq &_workq;
        io_uring ring{};

        // must be called with _sq_mutex held
        [[nodiscard]] auto _try_put_sq(io_uring_sqe *sqe) -> bool;

        static constexpr unsigned _max_queue_size = 512;
        std::deque<io_uring_sqe *> _pending;
    };

    static_assert(reactor<io_uring_reactor>);

}; // namespace sk::cio::posix

#endif // SK_CIO_POSIX_IO_URING_REACTOR_HXX_INCLUDED

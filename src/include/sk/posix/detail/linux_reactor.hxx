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

#ifndef SK_POSIX_DETAIL_LINUX_REACTOR_HXX_INCLUDED
#define SK_POSIX_DETAIL_LINUX_REACTOR_HXX_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sk/expected.hxx>
#include <sk/posix/detail/epoll_reactor.hxx>

#include <sk/detail/platform.hxx>

#ifdef SK_CIO_HAVE_IO_URING
#    include <sk/posix/detail/io_uring_reactor.hxx>
#endif

namespace sk::posix::detail {

    /*
     * A Linux reactor that splits I/O requests between epoll and io_uring.
     * Socket I/O (connect, accept, send, recv) always goes to the epoll
     * reactor; file I/O (open, close, read, write) goes to the io_uring
     * reactor if it's available, otherwise it goes to the epoll reactor for
     * thread dispatch.
     *
     * The reason we do this is that io_uring has a relatively low limit
     * on in-flight requests (about 512) meaning we can't queue blocking
     * i/os like socket reads, or else the queue will fill up and i/o will
     * deadlock.  The intent seems to be that io_uring will be used for
     * requests we expect to complete quickly, while epoll should still be
     * used for polling i/o.
     */
    struct linux_reactor final {
        linux_reactor() noexcept;
        linux_reactor(linux_reactor &&) noexcept = delete;
        auto operator=(linux_reactor &&) noexcept -> linux_reactor & = delete;
        virtual ~linux_reactor() = default;

        // Not copyable.
        linux_reactor(linux_reactor const &) = delete;
        auto operator=(linux_reactor const &) -> linux_reactor & = delete;

        // Associate a new fd with our epoll.
        [[nodiscard]] auto associate_fd(int fd) noexcept -> expected<void, std::error_code>;
        auto deassociate_fd(int fd) noexcept -> void;

        // Start this reactor.
        [[nodiscard]] auto start() noexcept -> expected<void, std::error_code>;

        // Stop this reactor.
        auto stop() noexcept -> void;

        auto get_system_executor() noexcept -> mt_executor *;

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        auto async_fd_open(char const *path, int flags, int mode = 0777) noexcept
            -> task<expected<int, std::error_code>>;

        auto async_fd_close(int fd) noexcept -> task<expected<int, std::error_code>>;

        auto async_fd_recv(int fd, void *buf, std::size_t n, int flags) noexcept
            -> task<expected<ssize_t, std::error_code>>;

        auto async_fd_read(int fd, void *buf, std::size_t n) noexcept
            -> task<expected<ssize_t, std::error_code>>;

        auto async_fd_pread(int fd, void *buf, std::size_t n, off_t offs) noexcept
            -> task<expected<ssize_t, std::error_code>>;

        auto async_fd_send(int fd, void const *buf, std::size_t n, int flags) noexcept
            -> task<expected<ssize_t, std::error_code>>;

        auto async_fd_write(int fd, void const *buf, std::size_t n) noexcept
            -> task<expected<ssize_t, std::error_code>>;

        auto async_fd_pwrite(int fd, void const *buf, std::size_t n, off_t offs) noexcept
            -> task<expected<ssize_t, std::error_code>>;

        auto async_fd_connect(int fd, sockaddr const *addr, socklen_t addrlen) noexcept
            -> task<expected<void, std::error_code>>;

        auto async_fd_accept(int fd, sockaddr *addr, socklen_t *addrlen) noexcept
            -> task<expected<int, std::error_code>>;

    private:
        std::unique_ptr<epoll_reactor> _epoll;
#ifdef SK_CIO_HAVE_IO_URING
        std::unique_ptr<io_uring_reactor> _uring;
#endif
    };

    inline linux_reactor::linux_reactor() noexcept = default;

    inline auto linux_reactor::get_system_executor() noexcept -> mt_executor *
    {
        static mt_executor xer;
        return &xer;
    }

    inline auto linux_reactor::associate_fd(int fd) noexcept -> expected<void, std::error_code>
    {
        return _epoll->associate_fd(fd);
    }

    inline auto linux_reactor::deassociate_fd(int fd) noexcept -> void
    {
        _epoll->deassociate_fd(fd);
    }

    inline auto linux_reactor::start() noexcept -> expected<void, std::error_code>
    {
        _epoll = std::make_unique<epoll_reactor>();

#ifdef SK_CIO_HAVE_IO_URING
        // An error returned means something went wrong trying to create the
        // uring reactor, e.g. out of memory.  A successful return of nullptr
        // means uring isn't supported on this system.
        auto ur = io_uring_reactor::make();
        if (!ur)
            return make_unexpected(ur.error());

        if (*ur != nullptr)
            _uring = std::move(*ur);
#endif

        get_system_executor()->start_threads();

        auto ret = _epoll->start();
        if (!ret)
            return make_unexpected(ret.error());

#ifdef SK_CIO_HAVE_IO_URING
        if (_uring) {
            ret = _uring->start();
            if (!ret)
                return make_unexpected(ret.error());
        }
#endif

        return {};
    }

    inline auto linux_reactor::stop() noexcept -> void
    {
        _epoll->stop();
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            _uring->stop();
#endif
    }

    /*
     * File I/O functions.
     */

    inline auto
    linux_reactor::async_fd_open(const char *path, int flags, int mode) noexcept
        -> task<expected<int, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_open(path, flags, mode);
#endif
        return _epoll->async_fd_open(path, flags, mode);
    }

    inline auto linux_reactor::async_fd_close(int fd) noexcept
        -> task<expected<int, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_close(fd);
#endif
        return _epoll->async_fd_close(fd);
    }

    inline auto linux_reactor::async_fd_read(int fd, void *buf, std::size_t n) noexcept
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_read(fd, buf, n);
#endif
        return _epoll->async_fd_read(fd, buf, n);
    }

    inline auto
    linux_reactor::async_fd_pread(int fd, void *buf, std::size_t n, off_t offs) noexcept
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_pread(fd, buf, n, offs);
#endif
        return _epoll->async_fd_pread(fd, buf, n, offs);
    }

    inline auto
    linux_reactor::async_fd_write(int fd, const void *buf, std::size_t n) noexcept
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_write(fd, buf, n);
#endif
        return _epoll->async_fd_write(fd, buf, n);
    }

    inline auto linux_reactor::async_fd_pwrite(int fd,
                                               const void *buf,
                                               std::size_t n,
                                               off_t offs) noexcept
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_pwrite(fd, buf, n, offs);
#endif
        return _epoll->async_fd_pwrite(fd, buf, n, offs);
    }

    /*
     * Socket I/O functions.  Go directly to epoll and do not pass io_uring.
     */

    inline auto
    linux_reactor::async_fd_recv(int fd, void *buf, std::size_t n, int flags) noexcept
        -> task<expected<ssize_t, std::error_code>>
    {
        return _epoll->async_fd_recv(fd, buf, n, flags);
    }

    inline auto linux_reactor::async_fd_send(int fd,
                                             const void *buf,
                                             std::size_t n,
                                             int flags) noexcept
        -> task<expected<ssize_t, std::error_code>>
    {
        return _epoll->async_fd_send(fd, buf, n, flags);
    }

    inline auto linux_reactor::async_fd_connect(int fd,
                                                const sockaddr *addr,
                                                socklen_t addrlen) noexcept
        -> task<expected<void, std::error_code>>
    {
        return _epoll->async_fd_connect(fd, addr, addrlen);
    }

    inline auto
    linux_reactor::async_fd_accept(int fd, sockaddr *addr, socklen_t *addrlen) noexcept
        -> task<expected<int, std::error_code>>
    {
        return _epoll->async_fd_accept(fd, addr, addrlen);
    }

} // namespace sk::posix::detail

#endif // SK_POSIX_DETAIL_LINUX_REACTOR_HXX_INCLUDED

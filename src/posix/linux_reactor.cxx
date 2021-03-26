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

#include <sk/cio/detail/config.hxx>
#include <sk/cio/posix/linux_reactor.hxx>

namespace sk::cio::posix {

    linux_reactor::linux_reactor()
    {
        _epoll = std::make_unique<epoll_reactor>(_workq);

#ifdef SK_CIO_HAVE_IO_URING
        _uring = io_uring_reactor::make(_workq);
#endif
    }

    auto linux_reactor::associate_fd(int fd) -> void
    {
        _epoll->associate_fd(fd);
    }

    auto linux_reactor::deassociate_fd(int fd) -> void
    {
        _epoll->deassociate_fd(fd);
    }

    auto linux_reactor::start() -> void
    {
        _epoll->start();
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            _uring->start();
#endif
    }

    auto linux_reactor::stop() -> void
    {
        _epoll->stop();
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            _uring->stop();
#endif
    }

    auto linux_reactor::post(std::function<void()> &&fn) -> void
    {
        _workq.post(std::move(fn));
    }

    /*
     * File I/O functions.
     */

    auto linux_reactor::async_fd_open(const char *path, int flags, int mode)
        -> task<expected<int, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_open(path, flags, mode);
#endif
        return _epoll->async_fd_open(path, flags, mode);
    }

    auto linux_reactor::async_fd_close(int fd)
        -> task<expected<int, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_close(fd);
#endif
        return _epoll->async_fd_close(fd);
    }


    auto linux_reactor::async_fd_read(int fd, void *buf, std::size_t n)
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_read(fd, buf, n);
#endif
        return _epoll->async_fd_read(fd, buf, n);
    }

    auto
    linux_reactor::async_fd_pread(int fd, void *buf, std::size_t n, off_t offs)
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_pread(fd, buf, n, offs);
#endif
        return _epoll->async_fd_pread(fd, buf, n, offs);
    }

    auto linux_reactor::async_fd_write(int fd, const void *buf, std::size_t n)
        -> task<expected<ssize_t, std::error_code>>
    {
#ifdef SK_CIO_HAVE_IO_URING
        if (_uring)
            return _uring->async_fd_write(fd, buf, n);
#endif
        return _epoll->async_fd_write(fd, buf, n);
    }

    auto linux_reactor::async_fd_pwrite(int fd,
                                        const void *buf,
                                        std::size_t n,
                                        off_t offs)
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

    auto
    linux_reactor::async_fd_recv(int fd, void *buf, std::size_t n, int flags)
    -> task<expected<ssize_t, std::error_code>>
    {
        return _epoll->async_fd_recv(fd, buf, n, flags);
    }

    auto linux_reactor::async_fd_send(int fd,
                                      const void *buf,
                                      std::size_t n,
                                      int flags)
    -> task<expected<ssize_t, std::error_code>>
    {
        return _epoll->async_fd_send(fd, buf, n, flags);
    }

    auto linux_reactor::async_fd_connect(int fd,
                                         const sockaddr *addr,
                                         socklen_t addrlen)
        -> task<expected<void, std::error_code>>
    {
        return _epoll->async_fd_connect(fd, addr, addrlen);
    }

    auto linux_reactor::async_fd_accept(int fd, sockaddr *addr, socklen_t *addrlen)
        -> task<expected<int, std::error_code>>
    {
        return _epoll->async_fd_accept(fd, addr, addrlen);
    }

} // namespace sk::cio::posix

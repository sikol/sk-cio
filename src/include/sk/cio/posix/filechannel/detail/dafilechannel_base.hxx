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

#ifndef SK_CIO_POSIX_FILECHANNEL_DAFILECHANNEL_BASE_HXX_INCLUDED
#define SK_CIO_POSIX_FILECHANNEL_DAFILECHANNEL_BASE_HXX_INCLUDED

#include <sys/types.h>
#include <fcntl.h>

#include <filesystem>
#include <system_error>

#include <sk/buffer/buffer.hxx>
#include <sk/cio/async_invoke.hxx>
#include <sk/cio/channel/concepts.hxx>
#include <sk/cio/detail/safeint.hxx>
#include <sk/cio/error.hxx>
#include <sk/cio/filechannel/filechannel.hxx>
#include <sk/cio/posix/async_api.hxx>
#include <sk/cio/posix/error.hxx>
#include <sk/cio/posix/fd.hxx>
#include <sk/cio/reactor.hxx>
#include <sk/cio/task.hxx>
#include <sk/cio/types.hxx>
#include <sk/cio/posix/filechannel/detail/filechannel_base.hxx>

namespace sk::cio::posix::detail {

    /*************************************************************************
     *
     * dafilechannel_base: base class for direct access file channels.
     *
     */

    struct dafilechannel_base : filechannel_base {
        using value_type = std::byte;

        dafilechannel_base(dafilechannel_base const &) = delete;
        dafilechannel_base(dafilechannel_base &&) noexcept = default;
        dafilechannel_base &operator=(dafilechannel_base const &) = delete;
        dafilechannel_base &operator=(dafilechannel_base &&) noexcept = default;


    protected:
        /*
         * Read data.
         */
        [[nodiscard]] auto
        _async_read_some_at(io_offset_t loc, std::byte *buffer, io_size_t nobjs)
            -> task<expected<io_size_t, std::error_code>>;

        [[nodiscard]] auto
        _read_some_at(io_offset_t loc, std::byte *buffer, io_size_t nobjs)
            -> expected<io_size_t, std::error_code>;

        /*
         * Write data.
         */
        [[nodiscard]] auto
        _async_write_some_at(io_offset_t, std::byte const *, io_size_t)
            -> task<expected<io_size_t, std::error_code>>;

        [[nodiscard]] auto
        _write_some_at(io_offset_t, std::byte const *, io_size_t)
            -> expected<io_size_t, std::error_code>;

        dafilechannel_base() = default;
        ~dafilechannel_base() = default;
    };

    /*************************************************************************
     * filechannel_base::_async_read_some_at()
     */

    inline auto dafilechannel_base::_async_read_some_at(io_offset_t loc,
                                                      std::byte *buffer,
                                                      io_size_t nobjs)
        -> task<expected<io_size_t, std::error_code>>
    {
        SK_CHECK(is_open(), "attempt to read on a closed channel");

        auto ret = co_await async_fd_pread(_fd.fd(), buffer, nobjs, loc);

        if (!ret)
            co_return make_unexpected(ret.error());

        if (*ret == 0)
            co_return make_unexpected(cio::error::end_of_file);

        co_return *ret;
    }

    /*************************************************************************
     * filechannel_base::_read_some_at()
     */

    inline auto dafilechannel_base::_read_some_at(io_offset_t loc,
                                                std::byte *buffer,
                                                io_size_t nobjs)
        -> expected<io_size_t, std::error_code>
    {
        SK_CHECK(is_open(), "attempt to read on a closed channel");

        auto ret = ::pread(_fd.fd(), buffer, nobjs, loc);

        if (ret == -1)
            return make_unexpected(get_errno());

        if (ret == 0)
            return make_unexpected(cio::error::end_of_file);

        return ret;
    }

    /*************************************************************************
     * odafilechannel::async_write_some_at()
     */

    inline auto dafilechannel_base::_async_write_some_at(io_offset_t loc,
                                                       std::byte const *buffer,
                                                       io_size_t nobjs)
        -> task<expected<io_size_t, std::error_code>>
    {
        SK_CHECK(is_open(), "attempt to read on a closed channel");

        auto ret = co_await async_fd_pwrite(_fd.fd(), buffer, nobjs, loc);

        if (!ret)
            co_return make_unexpected(ret.error());

        co_return ret;
    }

    /*************************************************************************
     * idafilechannel::write_some_at()
     */

    inline auto dafilechannel_base::_write_some_at(io_offset_t loc,
                                                 std::byte const *buffer,
                                                 io_size_t nobjs)
        -> expected<io_size_t, std::error_code>
    {
        SK_CHECK(is_open(), "attempt to read on a closed channel");

        auto ret = ::pwrite(_fd.fd(), buffer, nobjs, loc);

        if (!ret)
            return make_unexpected(get_errno());

        return ret;
    }

} // namespace sk::cio::posix::detail

#endif // SK_CIO_POSIX_FILECHANNEL_DAFILECHANNEL_BASE_HXX_INCLUDED

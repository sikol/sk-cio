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

#ifndef SK_CIO_WIN32_CHANNEL_SEQFILECHANNEL_HXX_INCLUDED
#define SK_CIO_WIN32_CHANNEL_SEQFILECHANNEL_HXX_INCLUDED

#include <cstddef>
#include <filesystem>
#include <system_error>

#include <sk/buffer/buffer.hxx>
#include <sk/cio/channel/concepts.hxx>
#include <sk/cio/channel/filechannel.hxx>
#include <sk/cio/error.hxx>
#include <sk/cio/task.hxx>
#include <sk/cio/types.hxx>
#include <sk/cio/win32/channel/detail/filechannel_base.hxx>
#include <sk/cio/win32/channel/detail/iseqfilechannel_base.hxx>
#include <sk/cio/win32/channel/detail/oseqfilechannel_base.hxx>
#include <sk/cio/win32/error.hxx>
#include <sk/cio/win32/handle.hxx>
#include <sk/cio/win32/iocp_reactor.hxx>

namespace sk::cio::win32 {

    /*************************************************************************
     *
     * seqfilechannel: a direct access channel that writes to a file.
     */

    // clang-format off
    struct seqfilechannel final 
            : detail::filechannel_base<seqfilechannel>
            , detail::iseqfilechannel_base<seqfilechannel>
            , detail::oseqfilechannel_base<seqfilechannel> {

        /*
         * Create a seqfilechannel which is closed.
         */
        seqfilechannel() = default;

        /*
         * Open a file.
         */
        [[nodiscard]]
        auto async_open(std::filesystem::path const &,
                        fileflags_t = fileflags::none) 
            -> task<expected<void, std::error_code>>;

        [[nodiscard]]
        auto open(std::filesystem::path const &,
                  fileflags_t = fileflags::none) 
            -> expected<void, std::error_code>;

        seqfilechannel(seqfilechannel const &) = delete;
        seqfilechannel(seqfilechannel &&) noexcept = default;
        seqfilechannel &operator=(seqfilechannel const &) = delete;
        seqfilechannel &operator=(seqfilechannel &&) noexcept = default;
        ~seqfilechannel() = default;
    };

    // clang-format on

    static_assert(seqchannel<seqfilechannel>);

    /*************************************************************************
     * seqfilechannel::async_open()
     */
    inline auto seqfilechannel::async_open(std::filesystem::path const &path,
                                           fileflags_t flags)
        -> task<expected<void, std::error_code>> {

        flags |= fileflags::read | fileflags::write;
        co_return co_await this->_async_open(path, flags);
    }

    /*************************************************************************
     * seqfilechannel::open()
     */
    inline auto seqfilechannel::open(std::filesystem::path const &path,
                                     fileflags_t flags)
        -> expected<void, std::error_code> {

        flags |= fileflags::read | fileflags::write;
        return this->_open(path, flags);
    }

} // namespace sk::cio::win32

#endif // SK_CIO_WIN32_CHANNEL_SEQFILECHANNEL_HXX_INCLUDED
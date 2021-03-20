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

#ifndef SK_CIO_CHANNEL_READ_HXX_INCLUDED
#define SK_CIO_CHANNEL_READ_HXX_INCLUDED

#include <sk/cio/channel/concepts.hxx>

namespace sk::cio {

    // clang-format off

    /*************************************************************************
     *
     * Channel input.
     *
     */

    /*************************************************************************
     * read_some()
     */

    template<iseqchannel Channel, sk::writable_buffer Buffer>
    auto read_some(Channel &channel,
                   io_size_t n, 
                   Buffer &buffer)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                  sk::buffer_value_t<Buffer>> {

        return channel.read_some(n, buffer);
    }

    template<iseqchannel Channel, sk::writable_buffer Buffer>
    auto async_read_some(Channel &channel,
                         io_size_t n, 
                         Buffer &buffer)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>> {

        return channel.async_read_some(n, buffer);
    }

    template<iseqchannel Channel, std::ranges::contiguous_range Range>
    auto read_some(Channel &channel,
                   io_size_t n, 
                   Range &range)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>> {

        auto buffer = sk::make_writable_range_buffer(range);
        return read_some(channel, n, buffer);
    }


    template<iseqchannel Channel, std::ranges::contiguous_range Range>
    auto async_read_some(Channel &channel,
                         io_size_t n, 
                         Range &range)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>> {

        auto buffer = sk::make_writable_range_buffer(range);
        co_return co_await async_read_some(channel, n, buffer);
    }

    /*************************************************************************
     * read_some_at()
     */

    template<idachannel Channel, sk::writable_buffer Buffer>
    auto read_some_at(Channel &channel,
                      io_size_t n, 
                      io_offset_t loc,
                      Buffer &buffer)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                  sk::buffer_value_t<Buffer>> {

        return channel.read_some_at(n, loc, buffer);
    }

    template<idachannel Channel, sk::writable_buffer Buffer>
    auto async_read_some_at(Channel &channel,
                            io_size_t n, 
                            io_offset_t loc,
                            Buffer &buffer)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>> {

        return channel.async_read_some_at(n, loc, buffer);
    }

    template<idachannel Channel, std::ranges::contiguous_range Range>
    auto read_some_at(Channel &channel,
                      io_size_t n, 
                      io_offset_t loc,
                      Range &range)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>> {

        auto buffer = sk::make_writable_range_buffer(range);
        return read_some_at(channel, n, loc, buffer);
    }

    template<idachannel Channel, std::ranges::contiguous_range Range>
    auto async_read_some_at(Channel &channel,
                         io_size_t n, 
                         io_offset_t loc,
                         Range &range)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>> {

        auto buffer = sk::make_writable_range_buffer(range);
        co_return co_await async_read_some_at(channel, n, loc, buffer);
    }

    // clang-format on

} // namespace sk::cio

#endif // SK_CIO_CHANNEL_READ_HXX_INCLUDED

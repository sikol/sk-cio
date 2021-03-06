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

#ifndef SK_CIO_CHANNEL_WRITE_HXX_INCLUDED
#define SK_CIO_CHANNEL_WRITE_HXX_INCLUDED

#include <sk/buffer.hxx>
#include <sk/channel/concepts.hxx>
#include <sk/detail/safeint.hxx>

namespace sk {

    /*************************************************************************
     *
     * Channel output.
     *
     */

    /*************************************************************************
     * write_some()
     */

    // clang-format off
    template<oseqchannel Channel, std::ranges::contiguous_range Range>
    [[nodiscard]]
    auto write_some(Channel &channel,
                    Range &&range,
                    io_size_t n = unlimited)
         -> expected<io_size_t, std::error_code>
    requires std::same_as<channel_value_t<Channel>,
                          std::ranges::range_value_t<Range>>
    // clang-format on
    {
        std::span<std::add_const_t<channel_value_t<Channel>>> span(range);

        if (n < span.size())
            span = span.first(n);

        return channel.write_some(span);
    }

    // clang-format off
    template<oseqchannel Channel, std::ranges::contiguous_range Range>
    [[nodiscard]]
    auto async_write_some(Channel &channel,
                          Range &&range,
                          io_size_t n = unlimited)
         -> task<expected<io_size_t, std::error_code>>
    requires std::same_as<channel_value_t<Channel>,
                          std::ranges::range_value_t<Range>>
    // clang-format on
    {
        std::span<std::add_const_t<channel_value_t<Channel>>> span(range);

        if (n < span.size())
            span = span.first(n);

        co_return co_await channel.async_write_some(span);
    }

    // clang-format off
    template<oseqchannel Channel, readable_buffer Buffer>
    [[nodiscard]]
    auto write_some(Channel &channel,
                    Buffer &buffer,
                    io_size_t n = unlimited)
        -> expected<io_size_t, std::error_code>
    requires std::same_as<channel_value_t<Channel>,
                          sk::buffer_value_t<Buffer>>
    // clang-format on
    {
        auto ranges = buffer.get_readable_ranges();
        if (std::ranges::size(ranges) == 0u)
            return 0u;

        auto &first_range = *std::ranges::begin(ranges);
        std::span span(first_range);
        if (n < span.size())
            span = span.first(n);

        auto nwritten = write_some(channel, span);

        if (!nwritten)
            return make_unexpected(nwritten.error());

        buffer.discard(*nwritten);
        return *nwritten;
    }

    // clang-format off
    template<oseqchannel Channel, readable_buffer Buffer>
    [[nodiscard]]
    auto async_write_some(Channel &channel,
                          Buffer &buffer,
                          io_size_t n = unlimited)
         -> task<expected<io_size_t, std::error_code>>
    requires std::same_as<channel_value_t<Channel>,
                          sk::buffer_value_t<Buffer>>
    // clang-format on
    {
        auto ranges = buffer.readable_ranges();
        if (std::ranges::size(ranges) == 0u)
            co_return 0u;

        auto &first_range = *std::ranges::begin(ranges);
        std::span span(first_range);
        if (n < span.size())
            span = span.first(n);

        auto nwritten = co_await async_write_some(channel, span);

        if (!nwritten)
            co_return make_unexpected(nwritten.error());

        buffer.discard(*nwritten);
        co_return *nwritten;
    }

    /*************************************************************************
     * write_all()
     */

    // clang-format off
    template<oseqchannel Channel, std::ranges::contiguous_range Range>
    [[nodiscard]]
    auto write_all(Channel &channel,
                   Range &&range,
                   io_size_t n = unlimited)
         -> std::pair<io_size_t, std::error_code>
    requires std::same_as<channel_const_value_t<Channel>,
                          std::add_const_t<std::ranges::range_value_t<Range>>>
    // clang-format on
    {
        std::span span(range);
        if (n < span.size())
            span = span.first(n);

        io_size_t nwritten = 0;

        while (!span.empty()) {
            auto ret = channel.write_some(span);

            if (!ret)
                return {nwritten, ret.error()};

            nwritten += *ret;
            span = span.subspan(*ret);
        }

        return {nwritten, sk::error::no_error};
    }

    // clang-format off
    template <oseqchannel Channel, std::ranges::contiguous_range Range>
    [[nodiscard]]
    auto async_write_all(Channel &channel,
                         Range &&range,
                         io_size_t n = unlimited)
        -> task<std::pair<io_size_t, std::error_code>>
    requires std::same_as<channel_value_t<Channel>,
                          std::ranges::range_value_t<Range>>
    // clang-format on
    {
        std::span span(range);
        if (n < span.size())
            span = span.first(n);

        io_size_t nwritten = 0;

        while (!span.empty()) {
            auto ret = co_await channel.async_write_some(span);

            if (!ret)
                co_return {nwritten, ret.error()};

            nwritten += *ret;
            span = span.subspan(*ret);
        }

        co_return {nwritten, sk::error::no_error};
    }

    // clang-format off
    template<oseqchannel Channel, readable_buffer Buffer>
    [[nodiscard]]
    auto write_all(Channel &channel,
                   Buffer &buffer,
                   io_size_t n = unlimited)
        -> std::pair<io_size_t, std::error_code>
    requires std::same_as<channel_value_t<Channel>,
                          sk::buffer_value_t<Buffer>>
    // clang-format on
    {
        io_size_t nwritten = 0;

        std::pair<io_size_t, std::error_code> ret;

        for (auto &&range : buffer.readable_ranges()) {
            ret = write_all(channel, range, n - nwritten);
            nwritten += ret.first;

            if (ret.first == 0 || ret.second || nwritten == n)
                break;
        }

        buffer.discard(nwritten);
        return {nwritten, ret.second};
    }

    // clang-format off
    template<oseqchannel Channel, readable_buffer Buffer>
    [[nodiscard]]
    auto async_write_all(Channel &channel,
                         Buffer &buffer,
                         io_size_t n = unlimited)
        -> task<std::pair<io_size_t, std::error_code>>
    requires std::same_as<channel_value_t<Channel>,
                          sk::buffer_value_t<Buffer>>
    // clang-format on
    {
        io_size_t nwritten = 0;

        std::pair<io_size_t, std::error_code> ret;

        for (auto &&range : buffer.readable_ranges()) {
            ret = co_await async_write_all(channel, range, n - nwritten);
            nwritten += ret.first;

            if (ret.first == 0 || ret.second || nwritten == n)
                break;
        }

        buffer.discard(nwritten);
        co_return {nwritten, ret.second};
    }

    /*************************************************************************
     * write_some_at()
     */

    // clang-format off
    template<odachannel Channel, std::ranges::contiguous_range Range>
    [[nodiscard]]
    auto write_some_at(Channel &channel,
                       io_offset_t loc,
                       Range &&range,
                       io_size_t n = unlimited)
         -> expected<io_size_t, std::error_code>
    requires std::same_as<channel_value_t<Channel>,
                          std::ranges::range_value_t<Range>>
    // clang-format on
    {
        std::span<std::add_const_t<channel_value_t<Channel>>> span(range);
        if (n < span.size())
            span = span.first(n);

        return channel.write_some_at(loc, span);
    }

    // clang-format off
    template<odachannel Channel, std::ranges::contiguous_range Range>
    [[nodiscard]]
    auto async_write_some_at(Channel &channel,
                             io_offset_t loc,
                             Range &&range,
                             io_size_t n = unlimited)
         -> task<expected<io_size_t, std::error_code>>
    requires std::same_as<channel_value_t<Channel>,
                          std::ranges::range_value_t<Range>>
    // clang-format on
    {
        std::span<std::add_const_t<channel_value_t<Channel>>> span(range);
        if (n < span.size())
            span = span.first(n);

        co_return co_await channel.async_write_some_at(loc, span);
    }

    // clang-format off
    template<odachannel Channel, readable_buffer Buffer>
    [[nodiscard]]
    auto write_some_at(Channel &channel,
                       io_offset_t loc,
                       Buffer &buffer,
                       io_size_t n = unlimited)
        -> expected<io_size_t, std::error_code>
    requires std::same_as<channel_value_t<Channel>,
                          sk::buffer_value_t<Buffer>>
    // clang-format on
    {
        auto ranges = buffer.get_readable_ranges();
        if (std::ranges::size(ranges) == 0u)
            return 0u;

        auto &first_range = *std::ranges::begin(ranges);

        auto bytes_written = write_some_at(channel, loc, first_range, n);
        if (!bytes_written)
            return bytes_written.error();

        buffer.discard(*bytes_written);
        return *bytes_written;
    }

    // clang-format off
    template<odachannel Channel, readable_buffer Buffer>
    [[nodiscard]]
    auto async_write_some_at(Channel &channel,
                             io_offset_t loc,
                             Buffer &buffer,
                             io_size_t n = unlimited)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>
    // clang-format on
    {
        auto ranges = buffer.get_readable_ranges();
        if (std::ranges::size(ranges) == 0u)
            co_return 0u;

        auto &first_range = *std::ranges::begin(ranges);

        auto bytes_written =
            co_await async_write_some_at(channel, loc, first_range, n);
        if (!bytes_written)
            co_return bytes_written.error();

        buffer.discard(*bytes_written);
        co_return *bytes_written;
    }

} // namespace sk

#endif // SK_CIO_CHANNEL_READ_HXX_INCLUDED

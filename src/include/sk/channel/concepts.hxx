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

#ifndef SK_CIO_CHANNEL_CONCEPTS_HXX_INCLUDED
#define SK_CIO_CHANNEL_CONCEPTS_HXX_INCLUDED

#include <concepts>
#include <cstddef>
#include <span>

#include <sk/channel/types.hxx>
#include <sk/expected.hxx>
#include <sk/task.hxx>

namespace sk {

    // clang-format off

    // Internal concepts.
    namespace detail {

        template<typename T>
        concept bytesized_char =
                std::integral<T> &&
                sizeof(T) == sizeof(std::byte);

    } // namespace detail

    /*************************************************************************
     *
     * Channel concepts.
     *
     * A channel is an input and/or output device.  We divide channels two
     * different ways: by I/O direction (input-only, output-only or both)
     * and by access type (sequential, random access, or both).
     *
     * An input channel can be read from, and an output channel can be
     * written to.  A sequential channel can be read and written to at
     * "the end", a random access channel can be read and written to anywhere.
     *
     */

    /*
     * channel_base: a channel which could be either an input channel
     * or an output channel.  A channel reads or writes objects of a
     * particular type (T::value_type), which is usually a char type.
     *
     * Note that closing the channel returns std::error_code.  This is
     * because the channel may be not fully flushed until it's closed,
     * and the final flush could return an error.
     */
    template <typename Channel>
    concept channel_base = requires(Channel &channel) {
        // The type that the channel reads and writes.  This is usually a
        // char-like type, although it doesn't have to be.
        typename Channel::value_type;

        // Close the channel synchronously.
        { channel.close() }
            -> std::same_as<expected<void, std::error_code>>;

        // Close the channel asynchronously.
        { channel.async_close() }
            -> std::same_as<task<expected<void, std::error_code>>>;
    };

    /*************************************************************************
     *
     * Sequential access (seq) channels.
     *
     * A sequential channel can be read from and written to sequentially.
     *
     * The most common example of a sequential channel is a socket, or a hardware
     * device like a serial port, but files are also sequential channels when
     * reading and writing using the internal file pointer.
     *
     */

    /*************************************************************************
     * oseqchannel - a channel that can be written to sequentially.
     */

    template <typename Channel>
    concept oseqchannel =
        channel_base<Channel> &&
        requires(Channel &channel, std::span<typename Channel::value_type const> buf) {
            // Write data to the channel synchronously.
            { channel.write_some(buf) }
                -> std::same_as<expected<io_size_t, std::error_code>>;

            // Write data to the channel asynchronously.
            { channel.async_write_some(buf) }
                -> std::same_as<task<expected<io_size_t, std::error_code>>>;
        };

    /*************************************************************************
     * iseqchannel - a channel that can be read from sequentially.
     */
    template <typename Channel>
    concept iseqchannel =
        channel_base<Channel> &&
        requires(Channel &channel, std::span<typename Channel::value_type> const buf) {
            // Read from the channel synchronously.
            { channel.read_some(buf) }
                -> std::same_as<expected<io_size_t, std::error_code>>;

            // Read from the channel asynchronously.
            { channel.async_read_some(buf) }
                -> std::same_as<task<expected<io_size_t, std::error_code>>>;
        };

    /*************************************************************************
     * seqchannel - a sequential channel that supports both input and output.
     */
    template <typename Channel>
    concept seqchannel = iseqchannel<Channel> && oseqchannel<Channel>;

    /*************************************************************************
     *
     * Direct access (da) channels.
     *
     * A direct access channel can be read from and written to at any location.
     * The most common example of a direct access channel is a file, or a
     * physical storage device like a hard disk.
     *
     */

    /*************************************************************************
     * odachannel - a direct access channel that can be written to.
     */

    template <typename Channel>
    concept odachannel =
        channel_base<Channel> &&
        requires(Channel &channel,
                 io_offset_t offset,
                 std::span<typename Channel::value_type const> &buf) {

            // Write data to the channel synchronously.
            { channel.write_some_at(offset, buf) }
                -> std::same_as<expected<io_size_t, std::error_code>>;

            // Write data to the channel asynchronously.
            { channel.async_write_some_at(offset, buf) }
                -> std::same_as<task<expected<io_size_t, std::error_code>>>;
        };

    /*************************************************************************
     * idachannel - a direct access channel that can be read from.
     */
    template <typename Channel>
    concept idachannel =
        channel_base<Channel> &&
        requires(Channel &channel,
                 io_offset_t offset,
                 std::span<typename Channel::value_type> &buf) {

            // Read from the channel synchronously.
            { channel.read_some_at(offset, buf) }
                -> std::same_as<expected<io_size_t, std::error_code>>;

            // Read from the channel asynchronously.
            { channel.async_read_some_at(offset, buf) }
                -> std::same_as<task<expected<io_size_t, std::error_code>>>;
        };

    /*************************************************************************
     * dachannel - a direct access channel that supports both input and output.
     */
    template <typename Channel>
    concept dachannel = idachannel<Channel> && odachannel<Channel>;

    /*************************************************************************
     *
     * Channel utilities.
     */

    // Return the value type of a channel.
    template<typename Channel>
    using channel_value_t = typename std::remove_cvref_t<Channel>::value_type;

    // Return the const value type of a channel.
    template<typename Channel>
    using channel_const_value_t =
        typename std::add_const_t<channel_value_t<Channel>>;

    // clang-format on

} // namespace sk

#endif // SK_CIO_CHANNEL_CONCEPTS_HXX_INCLUDED

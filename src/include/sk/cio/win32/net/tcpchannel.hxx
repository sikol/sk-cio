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

#ifndef SK_CIO_WIN32_NET_TCPCHANNEL_HXX_INCLUDED
#define SK_CIO_WIN32_NET_TCPCHANNEL_HXX_INCLUDED

#include <cstddef>

#include <sk/cio/async_invoke.hxx>
#include <sk/cio/channel/concepts.hxx>
#include <sk/cio/expected.hxx>
#include <sk/cio/net/address.hxx>
#include <sk/cio/task.hxx>
#include <sk/cio/win32/async_api.hxx>
#include <sk/cio/win32/handle.hxx>

namespace sk::cio::win32::net {

    // clang-format off
    struct tcpchannel {
        using value_type = std::byte;
        using native_handle_type = unique_socket;

        tcpchannel();
        tcpchannel(unique_socket &&);
        tcpchannel(tcpchannel const &) = delete;
        tcpchannel(tcpchannel &&) noexcept = default;
        tcpchannel &operator=(tcpchannel const &) = delete;
        tcpchannel &operator=(tcpchannel &&) noexcept = default;
        ~tcpchannel() = default;

        /*
         * Test if this channel has been opened.
         */
        auto is_open() const -> bool;

        /*
         * Connect to a remote host.
         */
        [[nodiscard]]
        auto async_connect(cio::net::address const &addr)
            -> task<expected<void, std::error_code>>;

        [[nodiscard]]
        auto connect(cio::net::address const &addr)
            -> expected<void, std::error_code>;

        /*
         * Read data.
         */
        template <sk::writable_buffer Buffer>
        [[nodiscard]]
        auto async_read_some(io_size_t n, Buffer &buffer)
            -> task<expected<io_size_t, std::error_code>> 
            requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>;

        template <sk::writable_buffer Buffer>
        [[nodiscard]]
        auto read_some(io_size_t n, Buffer &buffer)
            -> expected<io_size_t, std::error_code> 
            requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>;

        /*
         * Write data.
         */
        template <sk::readable_buffer Buffer>
        [[nodiscard]]
        auto async_write_some(io_size_t, Buffer &buffer)
            -> task<expected<io_size_t, std::error_code>>
            requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>;

        template <sk::readable_buffer Buffer>
        [[nodiscard]]
        auto write_some(io_size_t, Buffer &buffer)
            -> expected<io_size_t, std::error_code>
            requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>;

        /*
         * Close the socket.
         */
        [[nodiscard]] 
        auto async_close() 
             -> task<expected<void, std::error_code>>;

        [[nodiscard]] 
        auto close() 
             -> expected<void, std::error_code>;

    private:
        native_handle_type _native_handle;
    };
    // clang-format on

    /*************************************************************************
     * tcpchannel::tcpchannel()
     */

    inline tcpchannel::tcpchannel() {}

    inline tcpchannel::tcpchannel(unique_socket &&sock)
        : _native_handle(std::move(sock)) {}

    /*************************************************************************
     * tcpchannel::is_open()
     */

    inline auto tcpchannel::is_open() const -> bool {
        return _native_handle;
    }

    /*************************************************************************
     * tcpchannel::close()
     */
    inline auto tcpchannel::close() -> expected<void, std::error_code> {

        if (!is_open())
            return make_unexpected(cio::error::channel_not_open);

        auto err = _native_handle.close();
        if (err)
            return make_unexpected(err);
        return {};
    }

    /*************************************************************************
     * tcpchannel::async_close()
     */
    inline auto tcpchannel::async_close()
        -> task<expected<void, std::error_code>> {

        auto err =
            co_await async_invoke([&]() { return _native_handle.close(); });

        if (err)
            co_return make_unexpected(err);

        co_return {};
    }

    /*************************************************************************
     * tcpchannel::async_connect()
     */
    inline auto tcpchannel::async_connect(cio::net::address const &addr)
        -> task<expected<void, std::error_code>> {

#ifdef SK_CIO_CHECKED
        if (is_open())
            throw cio::detail::checked_error(
                "attempt to re-connect a connected tcpchannel");
#endif

        auto sock = ::WSASocketW(addr.address_family(), SOCK_STREAM,
                                 IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (sock == INVALID_SOCKET)
            co_return make_unexpected(win32::get_last_winsock_error());

        unique_socket sock_(sock);

        auto ret = co_await win32::AsyncConnectEx(
            _native_handle.native_socket(),
            reinterpret_cast<sockaddr const *>(&addr.native_address),
            addr.native_address_length, nullptr, 0, nullptr);

        if (!ret)
            co_return make_unexpected(ret.error());

        _native_handle = std::move(sock_);
    }

    /*************************************************************************
     * tcpchannel::connect()
     */
    inline auto tcpchannel::connect(cio::net::address const &addr)
        -> expected<void, std::error_code> {

#ifdef SK_CIO_CHECKED
        if (is_open())
            throw cio::detail::checked_error(
                "attempt to re-connect a connected tcpchannel");
#endif

        auto sock = ::WSASocketW(addr.address_family(), SOCK_STREAM,
                                 IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (sock == INVALID_SOCKET)
            return make_unexpected(win32::get_last_winsock_error());

        unique_socket sock_(sock);

        auto ret = ::WSAConnect(
            _native_handle.native_socket(),
            reinterpret_cast<sockaddr const *>(&addr.native_address),
            addr.native_address_length, nullptr, nullptr, nullptr, nullptr);

        if (ret)
            return make_unexpected(win32::get_last_winsock_error());

        _native_handle = std::move(sock_);
    }

    /*************************************************************************
     * tcpchannel::async_read_some()
     */

    // clang-format off
    template <sk::writable_buffer Buffer>
    inline auto tcpchannel::async_read_some(io_size_t nobjs,
                                            Buffer &buffer)
        -> task<expected<io_size_t, std::error_code>> 
        requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>
    {
        // clang-format on
#ifdef SK_CIO_CHECKED
        if (!is_open())
            throw cio::detail::checked_error(
                "attempt to read from a closed channel");
#endif

        DWORD bytes_read = 0;

        auto ranges = buffer.writable_ranges();

        if (std::ranges::size(ranges) == 0)
            co_return make_unexpected(cio::error::no_space_in_buffer);

        auto first_range = *std::ranges::begin(ranges);
        auto buf_data = std::ranges::data(first_range);
        auto buf_size = std::ranges::size(first_range);

        // The maximum I/O size we can support.
        auto max_objs =
            static_cast<std::size_t>(std::numeric_limits<DWORD>::max());

        // Can't read more than max_objs.
        if (nobjs > max_objs)
            nobjs = max_objs;

        // Can't read more than we have buffer space for.
        if (nobjs > buf_size)
            nobjs = buf_size;

        // The number of bytes we'll read.
        DWORD dwbytes = static_cast<DWORD>(nobjs);

        auto ret = co_await win32::AsyncReadFile(
            reinterpret_cast<HANDLE>(_native_handle.native_socket()), buf_data,
            dwbytes, &bytes_read, 0);

        if (!ret)
            co_return make_unexpected(
                win32::win32_to_generic_error(ret.error()));

        // 0 bytes = client went away
        if (bytes_read == 0)
            co_return make_unexpected(cio::error::end_of_file);

        buffer.commit(bytes_read);
        co_return bytes_read;
    }

    /*************************************************************************
     * tcpchannel::read_some()
     */

    // clang-format off
    template <sk::writable_buffer Buffer>
    inline auto tcpchannel::read_some(io_size_t nobjs, Buffer &buffer)
        -> expected<io_size_t, std::error_code> 
        requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>
    {
        // clang-format on

#ifdef SK_CIO_CHECKED
        if (!is_open())
            throw cio::detail::checked_error(
                "attempt to read from a closed channel");
#endif

        DWORD bytes_read = 0;

        auto ranges = buffer.writable_ranges();

        if (std::ranges::size(ranges) == 0)
            return make_unexpected(cio::error::no_space_in_buffer);

        auto first_range = *std::ranges::begin(ranges);
        auto buf_data = std::ranges::data(first_range);
        auto buf_size = std::ranges::size(first_range);

        // The maximum I/O size we can support.
        auto max_objs =
            static_cast<std::size_t>(std::numeric_limits<DWORD>::max());

        // Can't read more than max_objs.
        if (nobjs > max_objs)
            nobjs = max_objs;

        // Can't read more than we have buffer space for.
        if (nobjs > buf_size)
            nobjs = buf_size;

        // The number of bytes we'll read.
        DWORD dwbytes = static_cast<DWORD>(nobjs);

        OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(overlapped));

        // Create an event for the OVERLAPPED.  We don't actually use the event
        // but it has to be present.
        auto event_handle = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (event_handle == nullptr)
            return make_unexpected(
                win32::win32_to_generic_error(win32::get_last_error()));

        unique_handle evt(event_handle);

        // Set the low bit on the event handle to prevent the completion packet
        // being queued to the iocp_reactor, which won't know what to do with
        // it.
        overlapped.hEvent = reinterpret_cast<HANDLE>(
            reinterpret_cast<std::uintptr_t>(event_handle) | 0x1);

        auto ret =
            ::ReadFile(reinterpret_cast<HANDLE>(_native_handle.native_socket()),
                       buf_data, dwbytes, &bytes_read, &overlapped);

        if (!ret && (::GetLastError() == ERROR_IO_PENDING))
            ret = ::GetOverlappedResult(_native_handle.native_socket(),
                                        &overlapped, &bytes_read, TRUE);

        if (!ret)
            return make_unexpected(
                win32::win32_to_generic_error(win32::get_last_error()));

        // 0 bytes = client went away
        if (bytes_read == 0)
            co_return make_unexpected(cio::error::end_of_file);

        buffer.commit(bytes_read);
        return bytes_read;
    }

    /*************************************************************************
     * tcpchannel::async_write_some()
     */

    // clang-format off
    template <sk::readable_buffer Buffer>
    auto tcpchannel::async_write_some(io_size_t nobjs,
                                      Buffer &buffer)
        -> task<expected<io_size_t, std::error_code>> 
        requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>
    {
        // clang-format on
#ifdef SK_CIO_CHECKED
        if (!this->is_open())
            throw cio::detail::checked_error(
                "attempt to write to a closed channel");
#endif

        DWORD bytes_written = 0;

        auto ranges = buffer.readable_ranges();

        if (std::ranges::size(ranges) == 0)
            co_return make_unexpected(cio::error::no_data_in_buffer);

        auto first_range = *std::ranges::begin(ranges);
        auto buf_data = std::ranges::data(first_range);
        auto buf_size = std::ranges::size(first_range);

        // The maximum I/O size we can support.
        static auto max_objs =
            static_cast<std::size_t>(std::numeric_limits<DWORD>::max());

        // Can't write more than max_objs.
        if (nobjs > max_objs)
            nobjs = max_objs;

        // Can't write more than we have buffer space for.
        if (nobjs > buf_size)
            nobjs = buf_size;

        // The number of bytes we'll write.
        DWORD dwbytes = static_cast<DWORD>(nobjs);

        auto ret = co_await win32::AsyncWriteFile(
            reinterpret_cast<HANDLE>(_native_handle.native_socket()), buf_data,
            dwbytes, &bytes_written, 0);

        if (!ret)
            co_return make_unexpected(win32::win32_to_generic_error(ret.error()));

        buffer.discard(bytes_written);
        co_return bytes_written;
    }

    /*************************************************************************
     * tcpchannel::write_some()
     */

    // clang-format off
    template <sk::readable_buffer Buffer>
    auto tcpchannel::write_some(io_size_t nobjs,
                                Buffer &buffer)
        -> expected<io_size_t, std::error_code> 
        requires std::same_as<sk::buffer_value_t<Buffer>, std::byte>
    {
        // clang-format on
#ifdef SK_CIO_CHECKED
        if (!this->is_open())
            throw cio::detail::checked_error(
                "attempt to write to a closed channel");
#endif

        DWORD bytes_written = 0;

        auto ranges = buffer.readable_ranges();

        if (std::ranges::size(ranges) == 0)
            return make_unexpected(cio::error::no_data_in_buffer);

        auto first_range = *std::ranges::begin(ranges);
        auto buf_data = std::ranges::data(first_range);
        auto buf_size = std::ranges::size(first_range);

        // The maximum I/O size we can support.
        static auto max_objs =
            static_cast<std::size_t>(std::numeric_limits<DWORD>::max());

        // Can't write more than max_objs.
        if (nobjs > max_objs)
            nobjs = max_objs;

        // Can't write more than we have buffer space for.
        if (nobjs > buf_size)
            nobjs = buf_size;

        // The number of bytes we'll write.
        DWORD dwbytes = static_cast<DWORD>(nobjs);

        OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(overlapped));

        // Create an event for the OVERLAPPED.  We don't actually use the event
        // but it has to be present.
        auto event_handle = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (event_handle == nullptr)
            return make_unexpected(
                win32::win32_to_generic_error(win32::get_last_error()));

        unique_handle evt(event_handle);

        // Set the low bit on the event handle to prevent the completion packet
        // being queued to the iocp_reactor, which won't know what to do with
        // it.
        overlapped.hEvent = reinterpret_cast<HANDLE>(
            reinterpret_cast<std::uintptr_t>(event_handle) | 0x1);

        auto ret = ::WriteFile(
            reinterpret_cast<HANDLE>(_native_handle.native_socket()), buf_data,
            dwbytes, &bytes_written, &overlapped);

        if (!ret && (::GetLastError() == ERROR_IO_PENDING))
            ret = ::GetOverlappedResult(
                reinterpret_cast<HANDLE>(_native_handle.native_socket()),
                &overlapped, &bytes_written, TRUE);

        if (!ret) {
            return make_unexpected(
                win32::win32_to_generic_error(win32::get_last_error()));
        }

        buffer.discard(bytes_written);
        return bytes_written;
    }

} // namespace sk::cio::win32::net

#endif // SK_CIO_WIN32_NET_TCPCHANNEL_HXX_INCLUDED

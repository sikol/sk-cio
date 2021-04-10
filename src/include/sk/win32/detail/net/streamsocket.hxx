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

#ifndef SK_WIN32_DETAIL_NET_STREAMSOCKET_HXX_INCLUDED
#define SK_WIN32_DETAIL_NET_STREAMSOCKET_HXX_INCLUDED

#include <cstddef>

#include <afunix.h>

#include <sk/async_invoke.hxx>
#include <sk/channel/concepts.hxx>
#include <sk/detail/safeint.hxx>
#include <sk/expected.hxx>
#include <sk/net/address.hxx>
#include <sk/task.hxx>
#include <sk/win32/async_api.hxx>
#include <sk/win32/handle.hxx>

namespace sk::win32::detail {

    template <int type, int protocol>
    struct streamsocket {
    protected:
        unique_socket _native_handle;

        streamsocket() = default;

        explicit streamsocket(unique_socket &&);

        streamsocket(streamsocket &&) noexcept = default;

        auto operator=(streamsocket &&) noexcept -> streamsocket & = default;

        ~streamsocket() = default;

        [[nodiscard]] auto _async_connect(int af, sockaddr const *, socklen_t)
            -> task<expected<void, std::error_code>>;

        [[nodiscard]] auto _connect(int af, sockaddr const *, socklen_t)
            -> expected<void, std::error_code>;

    public:
        using value_type = std::byte;
        using native_handle_type = unique_socket;

        streamsocket(streamsocket const &) = delete;

        auto operator=(streamsocket const &) -> streamsocket & = delete;

        [[nodiscard]] auto is_open() const -> bool;

        [[nodiscard]] auto async_read_some(value_type *buffer, io_size_t n)
            -> task<expected<io_size_t, std::error_code>>;

        [[nodiscard]] auto read_some(value_type *buffer, io_size_t n)
            -> expected<io_size_t, std::error_code>;

        [[nodiscard]] auto async_write_some(value_type const *buffer, io_size_t)
            -> task<expected<io_size_t, std::error_code>>;

        [[nodiscard]] auto write_some(value_type const *buffer, io_size_t)
            -> expected<io_size_t, std::error_code>;

        [[nodiscard]] auto async_close()
            -> task<expected<void, std::error_code>>;

        [[nodiscard]] auto close() -> expected<void, std::error_code>;
    };

    /*************************************************************************
     * streamsocket::streamsocket()
     */

    template <int type, int protocol>
    streamsocket<type, protocol>::streamsocket(unique_socket &&sock)
        : _native_handle(std::move(sock))
    {
    }

    /*************************************************************************
     * streamsocket::is_open()
     */

    template <int type, int protocol>
    auto streamsocket<type, protocol>::is_open() const -> bool
    {
        return _native_handle;
    }

    /*************************************************************************
     * streamsocket::close()
     */
    template <int type, int protocol>
    auto streamsocket<type, protocol>::close()
        -> expected<void, std::error_code>
    {

        if (!is_open())
            return make_unexpected(sk::error::channel_not_open);

        auto err = _native_handle.close();
        if (err)
            return make_unexpected(err);
        return {};
    }

    /*************************************************************************
     * streamsocket::async_close()
     */
    template <int type, int protocol>
    auto streamsocket<type, protocol>::async_close()
        -> task<expected<void, std::error_code>>
    {
        auto err =
            co_await async_invoke([&]() { return _native_handle.close(); });

        if (err)
            co_return make_unexpected(err);

        co_return {};
    }

    /*************************************************************************
     * streamsocket::async_connect()
     */
    template <int type, int protocol>
    auto streamsocket<type, protocol>::_async_connect(int af,
                                                      sockaddr const *addr,
                                                      socklen_t addrlen)
        -> task<expected<void, std::error_code>>
    {
        SK_CHECK(!is_open(), "attempt to re-connect an open channel");

        auto sock = ::WSASocketW(
            af, SOCK_STREAM, protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (sock == INVALID_SOCKET)
            co_return make_unexpected(win32::get_last_winsock_error());

        unique_socket sock_(sock);

        // Winsock requires binding the socket before we can use ConnectEx().
        // Bind it to the all-zeroes address.
        sockaddr_storage zero_address{};
        zero_address.ss_family = static_cast<ADDRESS_FAMILY>(af);

        if (::bind(sock,
                   reinterpret_cast<sockaddr *>(&zero_address),
                   sizeof(zero_address)) != 0)
            co_return make_unexpected(win32::get_last_winsock_error());

        auto reactor = get_weak_reactor_handle();
        auto aret = reactor->associate_handle(handle_cast<HANDLE>(sock));

        if (!aret)
            co_return make_unexpected(aret.error());

        auto ret = co_await win32::AsyncConnectEx(
            sock, addr, addrlen, nullptr, 0, nullptr);

        if (!ret)
            co_return make_unexpected(ret.error());

        _native_handle = std::move(sock_);
        co_return {};
    }

    /*************************************************************************
     * streamsocket::connect()
     */
    template <int type, int protocol>
    auto streamsocket<type, protocol>::_connect(int af,
                                                sockaddr const *addr,
                                                socklen_t addrlen)
        -> expected<void, std::error_code>
    {
        SK_CHECK(is_open(), "attempt to connect on a closed channel");

        auto sock = ::WSASocketW(
            af, SOCK_STREAM, protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (sock == INVALID_SOCKET)
            return make_unexpected(win32::get_last_winsock_error());

        unique_socket sock_(sock);

        // Winsock requires binding the socket before we can use ConnectEx().
        // Bind it to the all-zeroes address.
        sockaddr_storage zero_address{};
        zero_address.ss_family = static_cast<ADDRESS_FAMILY>(af);

        if (::bind(sock,
                   reinterpret_cast<sockaddr *>(&zero_address),
                   sizeof(zero_address)) != 0)
            return make_unexpected(win32::get_last_winsock_error());

        auto reactor = get_weak_reactor_handle();
        auto aret = reactor->associate_handle(handle_cast<HANDLE>(sock));

        if (!aret)
            return make_unexpected(aret.error());

        auto ret = ::WSAConnect(
            sock, addr, addrlen, nullptr, nullptr, nullptr, nullptr);

        if (ret)
            return make_unexpected(win32::get_last_winsock_error());

        _native_handle = std::move(sock_);
        return {};
    }

    /*************************************************************************
     * streamsocket::async_read_some()
     */

    template <int type, int protocol>
    auto streamsocket<type, protocol>::async_read_some(value_type *buf,
                                                       io_size_t nobjs)
        -> task<expected<io_size_t, std::error_code>>
    {
        SK_CHECK(is_open(), "attempt to read on a closed channel");

        auto dwbytes = sk::detail::int_cast<DWORD>(nobjs);

        DWORD bytes_read = 0;
        auto ret = co_await win32::AsyncReadFile(
            reinterpret_cast<HANDLE>(_native_handle.native_socket()),
            buf,
            dwbytes,
            &bytes_read,
            0);

        if (!ret)
            co_return make_unexpected(
                win32::win32_to_generic_error(ret.error()));

        // 0 bytes = client went away
        if (bytes_read == 0)
            co_return make_unexpected(sk::error::end_of_file);

        co_return bytes_read;
    }

    /*************************************************************************
     * streamsocket::read_some()
     */

    template <int type, int protocol>
    auto streamsocket<type, protocol>::read_some(value_type *buf,
                                                 io_size_t nobjs)
        -> expected<io_size_t, std::error_code>
    {
        SK_CHECK(is_open(), "attempt to read on a closed channel");

        DWORD bytes_read = 0;
        auto dwbytes = sk::detail::int_cast<DWORD>(nobjs);

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
                       buf,
                       dwbytes,
                       &bytes_read,
                       &overlapped);

        if (!ret && (::GetLastError() == ERROR_IO_PENDING))
            ret = ::GetOverlappedResult(
                reinterpret_cast<HANDLE>(_native_handle.native_socket()),
                &overlapped,
                &bytes_read,
                TRUE);

        if (!ret)
            return make_unexpected(
                win32::win32_to_generic_error(win32::get_last_error()));

        // 0 bytes = client went away
        if (bytes_read == 0)
            return make_unexpected(sk::error::end_of_file);

        return bytes_read;
    }

    /*************************************************************************
     * streamsocket::async_write_some()
     */

    template <int type, int protocol>
    auto streamsocket<type, protocol>::async_write_some(value_type const *buf,
                                                        io_size_t nobjs)
        -> task<expected<io_size_t, std::error_code>>
    {
        SK_CHECK(is_open(), "attempt to write on a closed channel");

        DWORD bytes_written = 0;
        auto dwbytes = sk::detail::int_cast<DWORD>(nobjs);

        auto ret = co_await win32::AsyncWriteFile(
            reinterpret_cast<HANDLE>(_native_handle.native_socket()),
            buf,
            dwbytes,
            &bytes_written,
            0);

        if (!ret)
            co_return make_unexpected(
                win32::win32_to_generic_error(ret.error()));

        co_return bytes_written;
    }

    /*************************************************************************
     * streamsocket::write_some()
     */

    template <int type, int protocol>
    auto streamsocket<type, protocol>::write_some(value_type const *buf,
                                                  io_size_t nobjs)
        -> expected<io_size_t, std::error_code>
    {
        SK_CHECK(is_open(), "attempt to write on a closed channel");

        auto dwbytes = sk::detail::int_cast<DWORD>(nobjs);

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

        DWORD bytes_written = 0;
        auto ret = ::WriteFile(
            reinterpret_cast<HANDLE>(_native_handle.native_socket()),
            buf,
            dwbytes,
            &bytes_written,
            &overlapped);

        if (!ret && (::GetLastError() == ERROR_IO_PENDING))
            ret = ::GetOverlappedResult(
                reinterpret_cast<HANDLE>(_native_handle.native_socket()),
                &overlapped,
                &bytes_written,
                TRUE);

        if (!ret)
            return make_unexpected(
                win32::win32_to_generic_error(win32::get_last_error()));

        return bytes_written;
    }

} // namespace sk::win32::detail

#endif // SK_WIN32_DETAIL_NET_STREAMSOCKET_HXX_INCLUDED

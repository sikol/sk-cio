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

#ifndef SK_CIO_WIN32_HANDLE_HXX_INCLUDED
#define SK_CIO_WIN32_HANDLE_HXX_INCLUDED

#include <system_error>

#include <sk/check.hxx>
#include <sk/expected.hxx>
#include <sk/win32/error.hxx>
#include <sk/win32/windows.hxx>

namespace sk::win32 {

    /*************************************************************************
     *
     * RAII wrapper for Windows handles.
     *
     * unique_handle: this handle can only have one owner.  The handle is not
     * copyable, but it can be moved.
     */

    struct unique_handle {
        // Create an empty unique_handle.
        unique_handle() noexcept
            : _native_handle(INVALID_HANDLE_VALUE), _is_valid(false)
        {
        }

        // Create a unique_handle from a native handle.
        explicit unique_handle(HANDLE handle_value_) noexcept
            : _native_handle(handle_value_), _is_valid(true)
        {
        }

        // Move construction.
        unique_handle(unique_handle &&other) noexcept
            : _native_handle(other._native_handle), _is_valid(true)
        {

            other._is_valid = false;
        }

        // Move assignment.
        unique_handle &operator=(unique_handle &&other) noexcept
        {
            if (this == &other)
                return *this;

            close();

            _native_handle = other._native_handle;
            other._is_valid = false;
            return *this;
        }

        // Destructor.
        ~unique_handle() noexcept
        {
            close();
        }

        // Not copyable.
        unique_handle(unique_handle const &) = delete;
        unique_handle &operator=(unique_handle const &) = delete;

        // Assign a new value to this handle.
        auto assign(HANDLE native_handle) noexcept -> void
        {
            close();
            _native_handle = native_handle;
            _is_valid = true;
        }

        // Close the handle.
        auto close() noexcept -> std::error_code
        {
            if (!_is_valid)
                return win32::error::success;

            _is_valid = false;
            if (::CloseHandle(_native_handle))
                return win32::error::success;
            else
                return win32::get_last_error();
        }

        // Test if we have a valid handle.
        operator bool() const noexcept
        {
            return _is_valid;
        }

        // Return the Win32 handle.
        auto native_handle() -> HANDLE
        {
            SK_CHECK(_is_valid, "attempt to access invalid handle");
            return _native_handle;
        }

    private:
        bool _is_valid;
        HANDLE _native_handle;
    };

    /*************************************************************************
     *
     * unique_socket: a unique handle to a Winsock socket.
     */

    struct unique_socket {
        unique_socket() noexcept : _native_socket(INVALID_SOCKET) {}

        explicit unique_socket(SOCKET native_socket) noexcept
            : _native_socket(native_socket)
        {
        }

        unique_socket(unique_socket &&other) noexcept
            : _native_socket(
                  std::exchange(other._native_socket, INVALID_SOCKET))
        {
        }

        unique_socket &operator=(unique_socket &&other) noexcept
        {
            if (this == &other)
                return *this;

            close();

            _native_socket =
                std::exchange(other._native_socket, INVALID_SOCKET);
            return *this;
        }

        ~unique_socket() noexcept
        {
            close();
        }

        // Not copyable.
        unique_socket(unique_socket const &) = delete;
        unique_socket &operator=(unique_socket const &) = delete;

        // Assign a new value to this socket.
        auto assign(SOCKET native_socket) noexcept -> void
        {
            close();
            _native_socket = native_socket;
        }

        // Close the handle.
        auto close() noexcept -> std::error_code
        {
            if (!*this)
                return win32::error::success;

            auto ret = ::closesocket(_native_socket);
            _native_socket = INVALID_SOCKET;

            if (ret == 0)
                return win32::error::success;
            else
                return win32::get_last_winsock_error();
        }

        // Test if we have a valid socket.
        operator bool() const noexcept
        {
            return _native_socket != INVALID_SOCKET;
        }

        // Return the socket.
        auto native_socket() -> SOCKET
        {
            SK_CHECK(*this, "attempt to access invalid socket");
            return _native_socket;
        }

    private:
        SOCKET _native_socket;
    };

} // namespace sk::win32

#endif // SK_CIO_WIN32_HANDLE_HXX_INCLUDED
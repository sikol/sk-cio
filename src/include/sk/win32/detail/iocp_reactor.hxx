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

#ifndef SK_CIO_WIN32_IOCP_REACTOR_HXX_INCLUDED
#define SK_CIO_WIN32_IOCP_REACTOR_HXX_INCLUDED

#include <iostream>
#include <system_error>
#include <thread>

#include <sk/win32/error.hxx>
#include <sk/win32/handle.hxx>
#include <sk/win32/windows.hxx>
#include <sk/task.hxx>
#include <sk/workq.hxx>

namespace sk::win32::detail {

    struct iocp_coro_state : OVERLAPPED {
        iocp_coro_state() : OVERLAPPED({}) {}
        bool was_pending;
        BOOL success;
        DWORD error;
        DWORD bytes_transferred;
        coroutine_handle<> coro_handle;
        std::mutex mutex;
    };

    struct iocp_reactor {

        iocp_reactor();

        // Not copyable.
        iocp_reactor(iocp_reactor const &) = delete;
        iocp_reactor &operator=(iocp_reactor const &) = delete;

        // Movable.
        iocp_reactor(iocp_reactor &&) noexcept = delete;
        iocp_reactor &operator=(iocp_reactor &&) noexcept = delete;

        unique_handle completion_port;

        // Associate a new handle with our i/o port.
        auto associate_handle(HANDLE) -> void;

        // Start this reactor.
        auto start() -> void;

        // Stop this reactor.
        auto stop() -> void;

        // Post work to the reactor's thread pool.
        auto post(std::function<void()> fn) -> void;

    private:
        workq _workq;
        void completion_thread_fn(void);
        std::jthread completion_thread;
    };

}; // namespace sk::win32::detail

#endif // SK_CIO_WIN32_IOCP_REACTOR_HXX_INCLUDED
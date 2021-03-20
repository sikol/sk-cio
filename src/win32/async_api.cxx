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

#include <sk/cio/reactor.hxx>
#include <sk/cio/win32/async_api.hxx>
#include <sk/cio/win32/iocp_reactor.hxx>
#include <sk/cio/win32/spawn.hxx>

namespace sk::cio::win32 {

    task<HANDLE> AsyncCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                  DWORD dwShareMode,
                                  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                  DWORD dwCreationDisposition,
                                  DWORD dwFlagsAndAttributes,
                                  HANDLE hTemplateFile) {

        co_return co_await spawn([&]() -> HANDLE {
            auto handle = ::CreateFileW(
                lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

            if (handle != INVALID_HANDLE_VALUE)
                reactor_handle::get_global_reactor().associate_handle(handle);

            return handle;
        });
    }

    task<HANDLE> AsyncCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess,
                                  DWORD dwShareMode,
                                  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                  DWORD dwCreationDisposition,
                                  DWORD dwFlagsAndAttributes,
                                  HANDLE hTemplateFile) {
        co_return co_await spawn([&]() -> HANDLE {
            auto handle = ::CreateFileA(
                lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

            if (handle != INVALID_HANDLE_VALUE)
                reactor_handle::get_global_reactor().associate_handle(handle);

            return handle;
        });
    }

    struct co_ReadFile_awaiter {
        HANDLE hFile;
        LPVOID lpBuffer;
        DWORD nNumberOfBytesToRead;
        LPDWORD lpNumberOfBytesRead;
        iocp_coro_state *overlapped;

        bool did_suspend = false;

        bool await_ready() {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> coro_handle_) {
            std::lock_guard lock(overlapped->mutex);
            overlapped->coro_handle = coro_handle_;

            overlapped->success =
                ::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                           lpNumberOfBytesRead, overlapped);

            if (!overlapped->success && (GetLastError() == ERROR_IO_PENDING))
                return did_suspend = true;

            return did_suspend = false;
        }

        std::error_code await_resume() {
            DWORD error;

            if (did_suspend) {
                error = overlapped->error;
                *lpNumberOfBytesRead = overlapped->bytes_transferred;
            } else {
                error = GetLastError();
            }

            return win32::make_win32_error(error);
        }
    };

    task<std::error_code> AsyncReadFile(HANDLE hFile, LPVOID lpBuffer,
                                        DWORD nNumberOfBytesToRead,
                                        LPDWORD lpNumberOfBytesRead,
                                        DWORD64 Offset) {

        iocp_coro_state overlapped;
        memset(&overlapped, 0, sizeof(OVERLAPPED));
        overlapped.Offset = static_cast<DWORD>(Offset & 0xFFFFFFFFUL);
        overlapped.OffsetHigh = static_cast<DWORD>(Offset >> 32);

        co_return co_await co_ReadFile_awaiter{
            hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
            &overlapped};
    }

} // namespace sk::cio::win32

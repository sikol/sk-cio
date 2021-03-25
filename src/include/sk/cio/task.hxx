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

#ifndef SK_CIO_TASK_HXX_INCLUDED
#define SK_CIO_TASK_HXX_INCLUDED

#include <chrono>
#include <concepts>
#include <coroutine>
#include <future>
#include <iostream>
#include <optional>
#include <thread>
#include <utility>

namespace sk::cio {

    template <typename P>
    struct task_final_awaiter {
        bool destroy_self{false};

        bool await_ready() noexcept
        {
            return false;
        }

        void await_resume() noexcept {
        }

        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<P> h) noexcept
        {
            if (destroy_self) {
                h.destroy();
                return std::noop_coroutine();
            }

            auto previous = h.promise().previous;
            if (previous) {
                return previous;
            } else {
                return std::noop_coroutine();
            }
        }
    };

    template <typename T>
    struct task_promise {
        auto get_return_object()
        {
            return std::coroutine_handle<task_promise<T>>::from_promise(*this);
        }

        std::suspend_always initial_suspend()
        {
            return {};
        }

        task_final_awaiter<task_promise<T>> final_suspend() noexcept
        {
            return {destroy_self};
        }

        void unhandled_exception()
        {
            throw;
        }

        void return_value(T const &value) noexcept(
            std::is_nothrow_copy_constructible_v<T>)
        {
            result = value;
        }

        void return_value(T &&value) noexcept(
            std::is_nothrow_move_constructible_v<T>)
        {
            result = std::move(value);
        }

        T result{};
        bool destroy_self{false};
        std::coroutine_handle<> previous;
    };

    template <>
    struct task_promise<void> {
        auto get_return_object()
        {
            return std::coroutine_handle<task_promise<void>>::from_promise(
                *this);
        }

        std::suspend_always initial_suspend()
        {
            return {};
        }

        task_final_awaiter<task_promise<void>> final_suspend() noexcept
        {
            return {destroy_self};
        }

        void unhandled_exception()
        {
            throw;
        }

        void return_void() noexcept
        {
        }

        std::coroutine_handle<> previous;
        bool destroy_self{false};
    };

    template <typename T>
    struct task {
        using promise_type = task_promise<T>;
        std::coroutine_handle<promise_type> coro_handle;

        task(std::coroutine_handle<promise_type> coro_handle_)
            : coro_handle(coro_handle_)
        {
        }

        task(task const &) = delete;
        task &operator=(task const &) = delete;
        task &operator=(task &&other) = delete;

        task(task &&other) noexcept
            : coro_handle(std::exchange(other.coro_handle, {}))
        {
        }

        ~task()
        {
            if (coro_handle)
                coro_handle.destroy();
        }

        void start()
        {
            coro_handle.resume();
        }
    };

    template <typename T>
    auto operator co_await(task<T> const &task_) noexcept
    {
        struct task_awaiter {
            task<T> const &task_;

            bool await_ready()
            {
                return false;
            }

            T await_resume()
            {
                return std::move(task_.coro_handle.promise().result);
            }

            auto await_suspend(std::coroutine_handle<> h)
            {
                task_.coro_handle.promise().previous = h;
                return task_.coro_handle;
            }
        };
        return task_awaiter{task_};
    }

    template <>
    inline auto operator co_await(task<void> const &task_) noexcept
    {
        struct task_awaiter {
            task<void> const &task_;

            bool await_ready()
            {
                return false;
            }

            void await_resume() {}

            auto await_suspend(std::coroutine_handle<> h)
            {
                task_.coro_handle.promise().previous = h;
                return task_.coro_handle;
            }
        };

        return task_awaiter{task_};
    }

} // namespace sk::cio

#endif // SK_CIO_TASK_HXX_INCLUDED

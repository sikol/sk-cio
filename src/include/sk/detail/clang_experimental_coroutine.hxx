// -*- C++ -*-
//===----------------------------- coroutine -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef SK_DETAIL_CLANG_EXPERIMENTAL_COROUTINE_HXX
#define SK_DETAIL_CLANG_EXPERIMENTAL_COROUTINE_HXX

/**
    experimental/coroutine synopsis

// C++next

namespace std {
namespace experimental {
inline namespace coroutines_v1 {

  // 18.11.1 coroutine traits
template <typename R, typename... ArgTypes>
class coroutine_traits;
// 18.11.2 coroutine handle
template <typename Promise = void>
class coroutine_handle;
// 18.11.2.7 comparison operators:
bool operator==(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator!=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator<(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator<=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator>=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator>(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
// 18.11.3 trivial awaitables
struct suspend_never;
struct suspend_always;
// 18.11.2.8 hash support:
template <class T> struct hash;
template <class P> struct hash<coroutine_handle<P>>;

} // namespace coroutines_v1
} // namespace experimental
} // namespace std

 */

#include <cstddef>
#include <functional>
#include <memory> // for hash<T*>
#include <new>
#include <type_traits>

#include <sk/check.hxx>

namespace std::experimental { // NOLINT

    template <class>
    struct __void_t {
        using type = void;
    };

    template <class _Tp, class = void>
    struct __coroutine_traits_sfinae {
    };

    template <class _Tp>
    struct __coroutine_traits_sfinae<
        _Tp,
        typename __void_t<typename _Tp::promise_type>::type> {
        using promise_type = typename _Tp::promise_type;
    };

    template <typename _Ret, typename... _Args>
    struct coroutine_traits : public __coroutine_traits_sfinae<_Ret> {
    };

    template <typename _Promise = void>
    class coroutine_handle;

    template <>
    class coroutine_handle<void> {
    public:
        constexpr coroutine_handle() noexcept = default;
        explicit constexpr coroutine_handle(std::nullptr_t) noexcept {}

        auto operator=(std::nullptr_t) noexcept -> coroutine_handle &
        {
            __handle_ = nullptr;
            return *this;
        }

        [[nodiscard]] constexpr auto address() const noexcept -> void *
        {
            return __handle_;
        }

        constexpr explicit operator bool() const noexcept
        {
            return __handle_ != nullptr;
        }

        void operator()()
        {
            resume();
        }

        void resume()
        {
            SK_CHECK(__is_suspended(),
                     "resume() can only be called on suspended coroutines");
            SK_CHECK(
                !done(),
                "resume() has undefined behavior when the coroutine is done");
            __builtin_coro_resume(__handle_);
        }

        void destroy()
        {
            SK_CHECK(__is_suspended(),
                     "destroy() can only be called on suspended coroutines");
            __builtin_coro_destroy(__handle_);
        }

        [[nodiscard]] auto done() const -> bool
        {
            SK_CHECK(__is_suspended(),
                     "done() can only be called on suspended coroutines");
            return __builtin_coro_done(__handle_);
        }

        static auto from_address(void *__addr) noexcept -> coroutine_handle
        {
            coroutine_handle __tmp;
            __tmp.__handle_ = __addr;
            return __tmp;
        }

        // FIXME: Should from_address(nullptr) be allowed?

        static auto from_address(std::nullptr_t) noexcept -> coroutine_handle
        {
            return coroutine_handle(nullptr);
        }

        template <class _Tp, bool _CallIsValid = false>
        static auto from_address(_Tp *) -> coroutine_handle
        {
            static_assert(
                _CallIsValid,
                "coroutine_handle<void>::from_address cannot be called with "
                "non-void pointers");
        }

    private:
        [[nodiscard]] auto __is_suspended() const noexcept -> bool
        {
            // FIXME actually implement a check for if the coro is suspended.
            return __handle_ != nullptr;
        }

        template <class _PromiseT>
        friend class coroutine_handle;
        void *__handle_ = nullptr;
    };

    // 18.11.2.7 comparison operators:
    inline auto operator==(coroutine_handle<> __x,
                           coroutine_handle<> __y) noexcept -> bool
    {
        return __x.address() == __y.address();
    }
    inline auto operator!=(coroutine_handle<> __x,
                           coroutine_handle<> __y) noexcept -> bool
    {
        return !(__x == __y);
    }
    inline auto operator<(coroutine_handle<> __x,
                          coroutine_handle<> __y) noexcept -> bool
    {
        return std::less<>()(__x.address(), __y.address());
    }
    inline auto operator>(coroutine_handle<> __x,
                          coroutine_handle<> __y) noexcept -> bool
    {
        return __y < __x;
    }
    inline auto operator<=(coroutine_handle<> __x,
                           coroutine_handle<> __y) noexcept -> bool
    {
        return !(__x > __y);
    }
    inline auto operator>=(coroutine_handle<> __x,
                           coroutine_handle<> __y) noexcept -> bool
    {
        return !(__x < __y);
    }

    template <typename _Promise>
    class coroutine_handle : public coroutine_handle<> {
        using _Base = coroutine_handle<>;

    public:
#ifndef _LIBCPP_CXX03_LANG
        // 18.11.2.1 construct/reset
        using coroutine_handle<>::coroutine_handle;
#else
        coroutine_handle() _NOEXCEPT : _Base() {}
        coroutine_handle(nullptr_t) _NOEXCEPT : _Base(nullptr) {}
#endif

        auto operator=(std::nullptr_t) noexcept -> coroutine_handle &
        {
            _Base::operator=(nullptr);
            return *this;
        }

        [[nodiscard]] auto promise() const -> _Promise &
        {
            return *static_cast<_Promise *>(__builtin_coro_promise(
                this->__handle_, alignof(_Promise), false));
        }

        [[nodiscard]] static auto from_address(void *__addr) noexcept -> coroutine_handle
        {
            coroutine_handle __tmp;
            __tmp.__handle_ = __addr;
            return __tmp;
        }

        // NOTE: this overload isn't required by the standard but is needed so
        // the deleted _Promise* overload doesn't make from_address(nullptr)
        // ambiguous.
        // FIXME: should from_address work with nullptr?

        static auto from_address(std::nullptr_t) noexcept -> coroutine_handle
        {
            return coroutine_handle(nullptr);
        }

        template <class _Tp, bool _CallIsValid = false>
        static auto from_address(_Tp *) -> coroutine_handle
        {
            static_assert(_CallIsValid,
                          "coroutine_handle<promise_type>::from_address cannot "
                          "be called with "
                          "non-void pointers");
        }

        template <bool _CallIsValid = false>
        static auto from_address(_Promise *) -> coroutine_handle
        {
            static_assert(_CallIsValid,
                          "coroutine_handle<promise_type>::from_address cannot "
                          "be used with "
                          "pointers to the coroutine's promise type; use "
                          "'from_promise' instead");
        }

        static auto from_promise(_Promise &__promise) noexcept -> coroutine_handle
        {
            using _RawPromise = typename std::remove_cv<_Promise>::type;
            coroutine_handle __tmp;
            __tmp.__handle_ = __builtin_coro_promise(
                std::addressof(const_cast<_RawPromise &>(__promise)),
                alignof(_Promise),
                true);
            return __tmp;
        }
    };

#if __has_builtin(__builtin_coro_noop)
    struct noop_coroutine_promise {
    };

    template <>
    class coroutine_handle<noop_coroutine_promise> : public coroutine_handle<> {
        using _Base = coroutine_handle<>;
        using _Promise = noop_coroutine_promise;

    public:
        [[nodiscard]] auto promise() const -> _Promise &
        {
            return *static_cast<_Promise *>(__builtin_coro_promise(
                this->__handle_, alignof(_Promise), false));
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return true;
        }
        [[nodiscard]] constexpr auto done() const noexcept -> bool
        {
            return false;
        }

        constexpr void operator()() const noexcept {}
        constexpr void resume() const noexcept {}
        constexpr void destroy() const noexcept {}

    private:
        friend auto
        noop_coroutine() noexcept -> coroutine_handle<noop_coroutine_promise>;

        coroutine_handle() noexcept
        {
            this->__handle_ = __builtin_coro_noop();
        }
    };

    using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

    inline auto noop_coroutine() noexcept -> noop_coroutine_handle
    {
        return noop_coroutine_handle();
    }
#endif // __has_builtin(__builtin_coro_noop)

    struct suspend_never {

        [[nodiscard]] auto await_ready() const noexcept -> bool
        {
            return true;
        }

        void await_suspend(coroutine_handle<>) const noexcept {}

        void await_resume() const noexcept {}
    };

    struct suspend_always {

        [[nodiscard]] auto await_ready() const noexcept -> bool
        {
            return false;
        }

        void await_suspend(coroutine_handle<>) const noexcept {}

        void await_resume() const noexcept {}
    };

} // namespace std::experimental

namespace std {

    template <class _Tp>
    struct hash<std::experimental::coroutine_handle<_Tp>> {
        using __arg_type = std::experimental::coroutine_handle<_Tp>;
        auto operator()(__arg_type const &__v) const noexcept -> size_t
        {
            return hash<void *>()(__v.address());
        }
    };

} // namespace std

#endif // SK_DETAIL_CLANG_EXPERIMENTAL_COROUTINE_HXX

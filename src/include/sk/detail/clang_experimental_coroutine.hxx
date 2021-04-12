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
    struct void_t {
        using type = void;
    };

    template <class Tp, class = void>
    struct coroutine_traits_sfinae {
    };

    template <class Tp>
    struct coroutine_traits_sfinae<
        Tp,
        typename void_t<typename Tp::promise_type>::type> {
        using promise_type = typename Tp::promise_type;
    };

    template <typename Ret, typename... Args>
    struct coroutine_traits : public coroutine_traits_sfinae<Ret> {
    };

    template <typename Promise = void>
    class coroutine_handle;

    template <>
    class coroutine_handle<void> {
    public:
        constexpr coroutine_handle() noexcept = default;
        explicit constexpr coroutine_handle(std::nullptr_t) noexcept {}

        auto operator=(std::nullptr_t) noexcept -> coroutine_handle &
        {
            _handle = nullptr;
            return *this;
        }

        [[nodiscard]] constexpr auto address() const noexcept -> void *
        {
            return _handle;
        }

        constexpr explicit operator bool() const noexcept
        {
            return _handle != nullptr;
        }

        void operator()()
        {
            resume();
        }

        void resume()
        {
            SK_CHECK(_is_suspended(),
                     "resume() can only be called on suspended coroutines");
            SK_CHECK(
                !done(),
                "resume() has undefined behavior when the coroutine is done");
            __builtin_coro_resume(_handle);
        }

        void destroy()
        {
            SK_CHECK(_is_suspended(),
                     "destroy() can only be called on suspended coroutines");
            __builtin_coro_destroy(_handle);
        }

        [[nodiscard]] auto done() const -> bool
        {
            SK_CHECK(_is_suspended(),
                     "done() can only be called on suspended coroutines");
            return __builtin_coro_done(_handle);
        }

        static auto from_address(void *addr) noexcept -> coroutine_handle
        {
            coroutine_handle tmp;
            tmp._handle = addr;
            return tmp;
        }

        // FIXME: Should from_address(nullptr) be allowed?

        static auto from_address(std::nullptr_t) noexcept -> coroutine_handle
        {
            return coroutine_handle(nullptr);
        }

        template <class Tp, bool CallIsValid = false>
        static auto from_address(Tp * /*address*/) -> coroutine_handle
        {
            static_assert(
                CallIsValid,
                "coroutine_handle<void>::from_address cannot be called with "
                "non-void pointers");
        }

    private:
        [[nodiscard]] auto _is_suspended() const noexcept -> bool
        {
            // FIXME actually implement a check for if the coro is suspended.
            return _handle != nullptr;
        }

        template <class PromiseT>
        friend class coroutine_handle;
        void *_handle = nullptr;
    };

    // 18.11.2.7 comparison operators:
    inline auto operator==(coroutine_handle<> x,
                           coroutine_handle<> y) noexcept -> bool
    {
        return x.address() == y.address();
    }
    inline auto operator!=(coroutine_handle<> x,
                           coroutine_handle<> y) noexcept -> bool
    {
        return !(x == y);
    }
    inline auto operator<(coroutine_handle<> x,
                          coroutine_handle<> y) noexcept -> bool
    {
        return std::less<>()(x.address(), y.address());
    }
    inline auto operator>(coroutine_handle<> x,
                          coroutine_handle<> y) noexcept -> bool
    {
        return y < x;
    }
    inline auto operator<=(coroutine_handle<> x,
                           coroutine_handle<> y) noexcept -> bool
    {
        return !(x > y);
    }
    inline auto operator>=(coroutine_handle<> x,
                           coroutine_handle<> y) noexcept -> bool
    {
        return !(x < y);
    }

    template <typename Promise>
    class coroutine_handle : public coroutine_handle<> {
        using Base = coroutine_handle<>;

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
            Base::operator=(nullptr);
            return *this;
        }

        [[nodiscard]] auto promise() const -> Promise &
        {
            return *static_cast<Promise *>(__builtin_coro_promise(
                this->_handle, alignof(Promise), false));
        }

        [[nodiscard]] static auto from_address(void *addr) noexcept -> coroutine_handle
        {
            coroutine_handle tmp;
            tmp._handle = addr;
            return tmp;
        }

        // NOTE: this overload isn't required by the standard but is needed so
        // the deleted _Promise* overload doesn't make from_address(nullptr)
        // ambiguous.
        // FIXME: should from_address work with nullptr?

        static auto from_address(std::nullptr_t) noexcept -> coroutine_handle
        {
            return coroutine_handle(nullptr);
        }

        template <class Tp, bool CallIsValid = false>
        static auto from_address(Tp * /*address*/) -> coroutine_handle
        {
            static_assert(CallIsValid,
                          "coroutine_handle<promise_type>::from_address cannot "
                          "be called with "
                          "non-void pointers");
        }

        template <bool CallIsValid = false>
        static auto from_address(Promise * /*address*/) -> coroutine_handle
        {
            static_assert(CallIsValid,
                          "coroutine_handle<promise_type>::from_address cannot "
                          "be used with "
                          "pointers to the coroutine's promise type; use "
                          "'from_promise' instead");
        }

        static auto from_promise(Promise &promise) noexcept -> coroutine_handle
        {
            using RawPromise = typename std::remove_cv<Promise>::type;
            coroutine_handle tmp;
            tmp._handle = __builtin_coro_promise(
                std::addressof(const_cast<RawPromise &>(promise)),
                alignof(Promise),
                true);
            return tmp;
        }
    };

#if __has_builtin(__builtin_coro_noop)
    struct noop_coroutine_promise {
    };

    template <>
    class coroutine_handle<noop_coroutine_promise> : public coroutine_handle<> {
        using Base = coroutine_handle<>;
        using Promise = noop_coroutine_promise;

    public:
        [[nodiscard]] auto promise() const -> Promise &
        {
            return *static_cast<Promise *>(__builtin_coro_promise(
                this->_handle, alignof(Promise), false));
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
            this->_handle = __builtin_coro_noop();
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

        void await_suspend(coroutine_handle<> /*handle*/) const noexcept {}

        void await_resume() const noexcept {}
    };

    struct suspend_always {

        [[nodiscard]] auto await_ready() const noexcept -> bool
        {
            return false;
        }

        void await_suspend(coroutine_handle<> /*handle*/) const noexcept {}

        void await_resume() const noexcept {}
    };

} // namespace std::experimental

namespace std {

    template <class Tp>
    struct hash<std::experimental::coroutine_handle<Tp>> {
        using arg_type = std::experimental::coroutine_handle<Tp>;
        auto operator()(arg_type const &v) const noexcept -> size_t
        {
            return hash<void *>()(v.address());
        }
    };

} // namespace std

#endif // SK_DETAIL_CLANG_EXPERIMENTAL_COROUTINE_HXX

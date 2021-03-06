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

#ifndef SK_DETAIL_SAFEINT_HXX
#define SK_DETAIL_SAFEINT_HXX

#include <algorithm>
#include <concepts>
#include <limits>

#include <sk/check.hxx>

namespace sk::detail {

    template <std::unsigned_integral To, std::unsigned_integral From>
    [[nodiscard]] auto int_cast(From v) -> To
    {
        if (v > std::numeric_limits<To>::max())
            v = std::numeric_limits<To>::max();

        return static_cast<To>(v);
    }

    template <std::unsigned_integral R,
              std::unsigned_integral T,
              std::unsigned_integral U>
    [[nodiscard]] auto clamped_max(T a, U b) -> R
    {
        auto a_ = int_cast<R>(a);
        auto b_ = int_cast<R>(b);
        return std::max(a_, b_);
    }

    template <std::unsigned_integral T, std::unsigned_integral U>
    [[nodiscard]] auto can_add(T a, U b) -> bool requires std::same_as<T, U>
    {
        T could_add = std::numeric_limits<T>::max() - a;
        return b <= could_add;
    }

    // clang-format off
    template <std::unsigned_integral T, std::unsigned_integral U,
              std::unsigned_integral R>
    [[nodiscard]] auto safe_add(T a, U b, R *r) -> bool
        requires std::same_as<T, U> && std::same_as<T, R> {
        // clang-format on

        if (!can_add(a, b))
            return false;

        *r = a + b;
        return true;
    }

    template<std::unsigned_integral To, std::unsigned_integral From>
    [[nodiscard]] constexpr auto widen(From v) -> To {
        static_assert(sizeof(To) > sizeof(From));
        return To(v);
    }

    template<std::signed_integral To, std::signed_integral From>
    [[nodiscard]] constexpr auto widen(From v) -> To {
        static_assert(sizeof(To) > sizeof(From));
        return To(v);
    }

    template <std::unsigned_integral To, std::unsigned_integral From>
    [[nodiscard]] constexpr auto narrow(From v) -> To
    {
        static_assert(sizeof(To) < sizeof(From));

        if (v > widen<From>(std::numeric_limits<To>::max()))
            sk::detail::unexpected("narrow() would truncate");

        return static_cast<To>(v);
    }

    template <std::signed_integral To, std::signed_integral From>
    [[nodiscard]] constexpr auto narrow(From v) -> To
    {
        static_assert(sizeof(To) < sizeof(From));

        if (v > widen<From>(std::numeric_limits<To>::max()) ||
            v < widen<From>(std::numeric_limits<To>::min()))
            sk::detail::unexpected("narrow() would truncate");

        return static_cast<To>(v);
    }

    template<std::unsigned_integral To, std::unsigned_integral From>
    [[nodiscard]] constexpr auto truncate(From v) -> To {
        static_assert(sizeof(To) < sizeof(From));

        if (v > widen<From>(std::numeric_limits<To>::max()))
            return std::numeric_limits<To>::max();

        return static_cast<To>(v);
    }

    template<std::signed_integral To, std::unsigned_integral From>
    [[nodiscard]] constexpr auto truncate(From v) -> To {
        static_assert(sizeof(To) <= sizeof(From));
        static_assert(std::numeric_limits<To>::max() < std::numeric_limits<From>::max());

        if (v > static_cast<From>(std::numeric_limits<To>::max()))
            return std::numeric_limits<To>::max();

        return static_cast<To>(v);
    }

} // namespace sk::detail

#endif // SK_DETAIL_SAFEINT_HXX

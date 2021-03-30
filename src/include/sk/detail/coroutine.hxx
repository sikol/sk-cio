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

#ifndef SK_DETAIL_COROUTINE_HXX
#define SK_DETAIL_COROUTINE_HXX

#if defined(__cpp_impl_coroutine)
// C++20 coroutines: GCC, MSVC.
#    define SK_HAS_STD_COROUTINES
#    include <coroutine>

namespace sk {

    using std::coroutine_handle;
    using std::noop_coroutine_handle;
    using std::suspend_always;
    using std::suspend_never;

} // namespace sk

#elif defined(__clang__)
// Clang: TS coroutines.

#    define SK_HAS_TS_COROUTINES

#    if defined(_MSC_VER) || !__has_include(<experimental/coroutine>)
// If <experimental/coroutine> is not present (e.g., libstdc++) then provide
// our own.  We also need to do this on Clang-cl because the Microsoft header
// isn't ABI-compatible with Clang's coroutines.
#        include <sk/detail/clang_experimental_coroutine.hxx>
#    else
#        include <experimental/coroutine>
#    endif

namespace sk {

    using std::experimental::coroutine_handle;
    using std::experimental::suspend_always;
    using std::experimental::suspend_never;

} // namespace sk

#else
#    error could not detect a supported coroutine implementation

#endif

#endif // SK_DETAIL_COROUTINE_HXX
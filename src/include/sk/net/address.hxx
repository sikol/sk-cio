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

#ifndef SK_CIO_NET_ADDRESS_HXX_INCLUDED
#define SK_CIO_NET_ADDRESS_HXX_INCLUDED

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <sk/detail/platform.hxx>
#include <sk/detail/safeint.hxx>
#include <sk/expected.hxx>
#include <sk/task.hxx>

#include <sk/net/address/address.hxx>
#include <sk/net/address/inet.hxx>
#include <sk/net/address/inet6.hxx>

#ifdef SK_CIO_PLATFORM_HAS_AF_UNIX
#include <sk/net/address/unix.hxx>
#endif

#include <sk/net/address/make_address.hxx>
#include <sk/net/address/resolve.hxx>

#endif // SK_CIO_NET_ADDRESS_HXX_INCLUDED

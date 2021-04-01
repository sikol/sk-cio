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

#include <catch.hpp>

#include <sk/net/address.hxx>
#include <sk/wait.hxx>

using namespace sk::net;
using sk::expected;

TEST_CASE("inet6_address: make_inet6_address") {
    auto addr = make_inet6_address("::1");
    REQUIRE(addr);
    REQUIRE(address_family(*addr) == AF_INET6);

    auto s = str(*addr);
    REQUIRE(s);
    REQUIRE(*s == "::1");

    addr = make_inet6_address("::1", 80);
    REQUIRE(addr);
    s = str(*addr);
    REQUIRE(s);
    REQUIRE(*s == "[::1]:80");

    addr = make_inet6_address("::");
    REQUIRE(addr);
    s = str(*addr);
    REQUIRE(s);
    REQUIRE(*s == "::");

    addr = make_inet6_address("1::2::3");
    REQUIRE(!addr);
    addr = make_inet6_address("127.0.0.1");
    REQUIRE(!addr);
}

TEST_CASE("inet6_address: address_cast to unspecified_address") {
    auto inet = make_inet6_address("::1", 80);
    REQUIRE(inet);

    auto unspec = address_cast<unspecified_address>(*inet);
    REQUIRE(unspec);
    REQUIRE(address_family(*unspec) == AF_INET6);

    auto s = str(*unspec);
    if (!s) {
        INFO(s.error().message());
        REQUIRE(s);
    }
    REQUIRE(*s == "[::1]:80");

    auto inet2 = address_cast<inet6_address>(*unspec);
    REQUIRE(address_family(*inet2) == AF_INET6);

    s = str(*inet2);
    REQUIRE(s);
    REQUIRE(*s == "[::1]:80");
}

TEST_CASE("inet6_address: make_unspecified_zero_address") {
    auto unspec_zero = make_unspecified_zero_address(AF_INET6);
    REQUIRE(*str(*unspec_zero) == "::");
    REQUIRE(unspec_zero);

    REQUIRE(address_family(*unspec_zero) == AF_INET6);

    auto inet_zero = address_cast<inet6_address>(*unspec_zero);
    REQUIRE(inet_zero);
    REQUIRE(address_family(*inet_zero) == AF_INET6);
    REQUIRE(*str(*inet_zero) == "::");
}

TEST_CASE("inet6_address: make_address with port") {
    auto addr = make_address("::1", "80");
    REQUIRE(addr);
    REQUIRE(*str(*addr) == "[::1]:80");

    auto iaddr = address_cast<inet6_address>(*addr);
    REQUIRE(iaddr);
    REQUIRE(*str(*iaddr) == "[::1]:80");
}

TEST_CASE("inet6_address: make_address without port") {
    auto addr = make_address("::1");
    REQUIRE(addr);
    REQUIRE(*str(*addr) == "::1");

    auto iaddr = address_cast<inet6_address>(*addr);
    REQUIRE(iaddr);
    REQUIRE(*str(*iaddr) == "::1");
}

TEST_CASE("inet6_address: streaming output") {
    auto addr = make_address("::1", "80");
    REQUIRE(addr);

    std::ostringstream strm;
    strm << *addr;
    REQUIRE(strm.str() == "[::1]:80");

}

TEST_CASE("inet6_address: resolve")
{
    expected<std::set<inet6_address>, std::error_code> addr =
        wait(async_resolve_address<AF_INET6>("localhost"));
    REQUIRE(addr);
    // >= because on some platforms localhost has multiple aliases.
    REQUIRE(addr->size() >= 1);

    auto &first = *addr->begin();

    REQUIRE(address_family(first) == AF_INET6);
    REQUIRE(*str(first) == "::1");
}

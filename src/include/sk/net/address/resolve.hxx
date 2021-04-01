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

#ifndef SK_NET_ADDRESS_RESOLVE_HXX_INCLUDED
#define SK_NET_ADDRESS_RESOLVE_HXX_INCLUDED

#include <array>
#include <set>

#include <sk/async_invoke.hxx>
#include <sk/net/address/address.hxx>
#include <sk/net/address/inet.hxx>
#include <sk/net/address/inet6.hxx>

namespace sk::net {

    /*************************************************************************
     *
     * The resolver error category.
     *
     */
    enum struct resolver_error : int {
        no_error = 0,
    };

} // namespace sk::net

namespace std {

    template <>
    struct is_error_code_enum<sk::net::resolver_error> : true_type {
    };

} // namespace std

namespace sk::net {

    namespace detail {

        struct resolver_errc_category : std::error_category {
            [[nodiscard]] auto name() const noexcept -> char const * final
            {
                return "cio:resolver";
            }
            [[nodiscard]] auto message(int c) const -> std::string final
            {
                return gai_strerror(c);
            }
        };

#ifdef NI_MAXHOST
        constexpr std::size_t ni_maxhost = NI_MAXHOST;
#else
        constexpr std::size_t ni_maxhost = 1025;
#endif

#ifdef NI_MAXSERV
        constexpr std::size_t ni_maxserv = NI_MAXSERV;
#else
        constexpr std::size_t ni_maxserv = 32;
#endif
    } // namespace detail

    [[nodiscard]] inline auto resolver_errc_category()
        -> detail::resolver_errc_category const &
    {
        static detail::resolver_errc_category c;
        return c;
    }

    [[nodiscard]] inline auto make_error_code(resolver_error e)
        -> std::error_code
    {
        return {static_cast<int>(e), resolver_errc_category()};
    }

    /*************************************************************************
     * Create an address from an address and service string.
     *
     * This does not attempt to resolve either argument, so they should
     * be literal strings.
     *
     * If only service is specified, the host in the return address will be
     * the "any" address.
     */
    [[nodiscard]] inline auto make_address(std::string const &host,
                                           std::string const &service = "")
        -> expected<unspecified_address, std::error_code>
    {
        addrinfo hints{};
        addrinfo *gai_result{};
        hints.ai_flags = AI_NUMERICHOST;

        auto ret = ::getaddrinfo(host.c_str(),
                                 service.empty() ? nullptr : service.c_str(),
                                 &hints,
                                 &gai_result);

        if (ret)
            return make_unexpected(
                make_error_code(static_cast<resolver_error>(ret)));

        auto addr = make_unspecified_address(
            gai_result->ai_addr,
            // On Windows, ai_addrlen is a size_t.
            static_cast<socklen_t>(gai_result->ai_addrlen));

        freeaddrinfo(gai_result);
        return addr;
    }

    /*************************************************************************
     *
     * async_resolve_address(): resolve a hostname to a list of addresses
     * using the operating system's resolver.  This is a naive implementation
     * that just uses spawns getaddrinfo() on another thread.
     *
     */
    template <int af = AF_UNSPEC>
    [[nodiscard]] auto async_resolve_address(std::string const &hostname,
                                             std::string const &port = "")
        -> task<expected<std::set<address<af>>, std::error_code>>
    {
        addrinfo hints{};
        addrinfo *gai_result{};

        static_assert(af == AF_INET || af == AF_INET6,
                      "async_resolve_address: invalid address family");

        hints.ai_family = af;

        auto ret = co_await async_invoke([&] {
            return ::getaddrinfo(hostname.c_str(),
                                 port.empty() ? nullptr : port.c_str(),
                                 &hints,
                                 &gai_result);
        });

        if (ret)
            co_return make_unexpected(
                make_error_code(static_cast<resolver_error>(ret)));

        std::unique_ptr<addrinfo, void (*)(addrinfo *)> gai_result_(
            gai_result, freeaddrinfo);

        std::set<address<af>> addresses;
        for (auto *p = gai_result; p; p = p->ai_next) {
            if (p->ai_family != af)
                continue;

            auto addr = address_cast<address<af>>(*p->ai_addr);
            if (!addr)
                co_return make_unexpected(addr.error());

            addresses.insert(*addr);
        }

        co_return addresses;
    }

    template <>
    [[nodiscard]] inline auto
    async_resolve_address<AF_UNSPEC>(std::string const &hostname,
                                     std::string const &port)
        -> task<expected<std::set<address<>>, std::error_code>>
    {
        addrinfo hints{};
        addrinfo *gai_result{};

        auto ret = co_await async_invoke([&] {
            return ::getaddrinfo(hostname.c_str(),
                                 port.empty() ? nullptr : port.c_str(),
                                 &hints,
                                 &gai_result);
        });

        if (ret)
            co_return make_unexpected(
                make_error_code(static_cast<resolver_error>(ret)));

        std::unique_ptr<addrinfo, void (*)(addrinfo *)> gai_result_(
            gai_result, freeaddrinfo);

        std::set<address<>> addresses;
        for (auto *p = gai_result; p; p = p->ai_next) {
            if (p->ai_family != AF_INET && p->ai_family != AF_INET6)
                continue;

            auto addr =
                make_unspecified_address(p->ai_addr,
                                         // On Windows, ai_addrlen is a size_t.
                                         static_cast<socklen_t>(p->ai_addrlen));
            if (!addr)
                co_return make_unexpected(addr.error());

            // Failure is okay, we don't return duplicate addresses.
            addresses.insert(*addr);
        }

        co_return addresses;
    }

    template <int address_family>
    inline auto operator<<(std::ostream &strm,
                           address<address_family> const &addr)
        -> std::ostream &
    {
        auto s = str(addr);

        if (s)
            strm << *s;
        else
            strm << "(invalid address)";

        return strm;
    }

} // namespace sk::net

#endif // SK_NET_ADDRESS_RESOLVE_HXX_INCLUDED

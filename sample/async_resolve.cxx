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

#include <cstdio>
#include <iostream>
#include <ranges>

#include <fmt/core.h>

#include <sk/cio.hxx>

using namespace sk;

task<void> resolve(std::string const &name) {
    std::cout << name << ": ";

    auto addresses = co_await net::async_resolve_address(name, "");
    if (!addresses) {
        std::cout << addresses.error().message() << '\n';
        co_return;
    }

    if (addresses->empty()) {
        std::cout << "no addresses\n";
        co_return;
    }

    std::cout << '\n';

    for (auto &&address : *addresses)
        std::cout << '\t' << address << '\n';

    std::cout << '\n';

}

int main(int argc, char **argv) {
    using namespace std::chrono_literals;

    if (argc < 2) {
        fmt::print(stderr, "usage: {} <file> [file...]", argv[0]);
        return 1;
    }

    sk::reactor_handle reactor;

    for (auto &&name : std::span(argv + 1, argv + argc)) {
        wait(resolve(name));
    }

    return 0;
}

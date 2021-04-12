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
#include <sk/co_main.hxx>

auto print_file(std::string const &name) -> sk::task<void> {
    sk::iseqfilechannel chnl;

    auto err = co_await chnl.async_open(name);
    if (!err) {
        std::cerr << name << ": " << err.error().message() << "\n";
        co_return;
    }

    auto cchnl = sk::make_iseqcharchannel<char>(std::move(chnl));

    for (;;) {
        sk::fixed_buffer<char, 1024> buf;
        auto nbytes = co_await sk::async_read_some(cchnl, buf);

        if (!nbytes) {
            if (nbytes.error() != sk::error::end_of_file)
                std::cerr << name << ": " << nbytes.error().message() << "\n";
            break;
        }

        for (auto &&range : buf.readable_ranges())
            std::cout.write(
                std::ranges::data(range),
                static_cast<std::streamsize>(std::ranges::size(range)));

        buf.discard(*nbytes);
    }

    co_await cchnl.async_close();
}

auto co_main(int argc, char **argv) -> sk::task<int> {
    using namespace std::chrono_literals;

    if (argc < 2) {
        fmt::print(stderr, "usage: {} <file> [file...]", argv[0]);
        co_return 1;
    }

    for (auto &&file : std::span(argv + 1, argv + argc)) {
        co_await print_file(file);
    }

    co_return 0;
}

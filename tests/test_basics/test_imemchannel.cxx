/*
 * Copyright(c) 2019, 2020, 2021 SiKol Ltd.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license(the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third - parties to whom the Software is furnished to
 * do so, all subject to the following :
 *
 * The copyright notices in the Softwareand this entire statement, including
 * the above license grant, this restrictionand the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine - executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <catch.hpp>

#include <cstring>

#include <sk/channel/read.hxx>
#include <sk/channel/memchannel/imemchannel.hxx>

using namespace sk;

TEST_CASE("imemchannel::read_some() partial buffer") {
    char buf[20] = {'A', 'B', 'C'};
    std::byte dat[4];
    std::memset(dat, 'X', sizeof dat);

    auto chnl = make_imemchannel(buf);

    auto nbytes = read_some(chnl, dat, 3);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 3);
    REQUIRE(dat[0] == std::byte{'A'});
    REQUIRE(dat[1] == std::byte{'B'});
    REQUIRE(dat[2] == std::byte{'C'});
    REQUIRE(dat[3] == std::byte{'X'});
}

TEST_CASE("imemchannel::read_some() entire buffer") {
    char inbuf[] = {'A', 'B', 'C' };
    std::byte outbuf[10];
    std::memset(outbuf, 'X', sizeof outbuf);

    auto chnl = make_imemchannel(inbuf);

    auto nbytes = read_some(chnl, outbuf);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 3);
    REQUIRE(outbuf[0] == std::byte{'A'});
    REQUIRE(outbuf[1] == std::byte{'B'});
    REQUIRE(outbuf[2] == std::byte{'C'});
    REQUIRE(outbuf[3] == std::byte{'X'});
}

TEST_CASE("imemchannel::read_some() single-byte") {
    char buf[3] = {'A', 'B', 'C'};
    std::byte dat[4];
    std::memset(dat, 'X', sizeof dat);

    auto chnl = make_imemchannel(buf);

    auto nbytes = read_some(chnl, dat, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = read_some(chnl, std::span(dat + 1, 1));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = read_some(chnl, std::span(dat + 2, 1));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = read_some(chnl, dat, 1);
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);

    REQUIRE(dat[0] == std::byte{'A'});
    REQUIRE(dat[1] == std::byte{'B'});
    REQUIRE(dat[2] == std::byte{'C'});
    REQUIRE(dat[3] == std::byte{'X'});
}

TEST_CASE("imemchannel::read_some_at() single-byte") {
    char buf[3] = {'A', 'B', 'C'};
    std::byte dat[4];
    std::memset(dat, 'X', sizeof dat);

    auto chnl = make_imemchannel(buf);

    auto nbytes = chnl.read_some_at(0, dat, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = chnl.read_some_at(1, dat + 1, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = chnl.read_some_at(2, dat + 2, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = chnl.read_some_at(3, dat, 1);
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);

    REQUIRE(dat[0] == std::byte{'A'});
    REQUIRE(dat[1] == std::byte{'B'});
    REQUIRE(dat[2] == std::byte{'C'});
    REQUIRE(dat[3] == std::byte{'X'});
}

TEST_CASE("imemchannel::read_some() past the end") {
    char buf[3] = {'A', 'B', 'C'};
    std::byte dat[4];
    std::memset(dat, 'X', sizeof dat);

    auto chnl = make_imemchannel(buf);

    auto nbytes = chnl.read_some(dat, 4);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 3);
    REQUIRE(dat[0] == std::byte{'A'});
    REQUIRE(dat[1] == std::byte{'B'});
    REQUIRE(dat[2] == std::byte{'C'});
    REQUIRE(dat[3] == std::byte{'X'});
}

TEST_CASE("imemchannel::read_some_at() with an invalid location") {
    char buf[3] = {'A', 'B', 'C'};
    std::byte dat[4];
    std::memset(dat, 'X', sizeof dat);

    auto chnl = make_imemchannel(buf);

    auto nbytes = chnl.read_some_at(4, dat, 1);
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);
}

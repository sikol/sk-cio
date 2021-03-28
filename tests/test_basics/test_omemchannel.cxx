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

#include <sk/channel/memchannel/omemchannel.hxx>
#include <sk/channel/read.hxx>
#include <sk/channel/write.hxx>
#include <sk/wait.hxx>

using namespace sk;

TEST_CASE("omemchannel::write_some()")
{
    std::byte const buf[] = {std::byte('A'), std::byte('B'), std::byte('C')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out);

    auto nbytes = write_some(chnl, buf);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 3);
    REQUIRE(out[0] == 'A');
    REQUIRE(out[1] == 'B');
    REQUIRE(out[2] == 'C');
    REQUIRE(out[3] == 'X');
}

TEST_CASE("omemchannel::async_write_some()")
{
    std::byte const buf[] = {std::byte('A'), std::byte('B'), std::byte('C')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out);

    auto nbytes = wait(async_write_some(chnl, std::span(buf)));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 3);
    REQUIRE(out[0] == 'A');
    REQUIRE(out[1] == 'B');
    REQUIRE(out[2] == 'C');
    REQUIRE(out[3] == 'X');
}

TEST_CASE("omemchannel::write_some() single byte")
{
    std::byte const buf[] = {std::byte('A'), std::byte('B'), std::byte('C')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out, out + 3);

    auto nbytes = write_some(chnl, buf, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = write_some(chnl, std::span(buf + 1, 1));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = write_some(chnl, std::span(buf + 2, 1));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = write_some(chnl, std::span(buf + 3, 1));
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);

    REQUIRE(out[0] == 'A');
    REQUIRE(out[1] == 'B');
    REQUIRE(out[2] == 'C');
    REQUIRE(out[3] == 'X');
}

TEST_CASE("omemchannel::async_write_some() single byte")
{
    std::byte const buf[] = {std::byte('A'), std::byte('B'), std::byte('C')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out, out + 3);

    auto nbytes = wait(async_write_some(chnl, std::span(buf), 1));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = wait(async_write_some(chnl, std::span(buf + 1, 1)));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = wait(async_write_some(chnl, std::span(buf + 2, 1)));
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = wait(async_write_some(chnl, std::span(buf + 3, 1)));
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);

    REQUIRE(out[0] == 'A');
    REQUIRE(out[1] == 'B');
    REQUIRE(out[2] == 'C');
    REQUIRE(out[3] == 'X');
}

TEST_CASE("omemchannel::write_some_at() single byte")
{
    std::byte const buf[] = {std::byte('A'), std::byte('B'), std::byte('C')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out, out + 3);

    auto nbytes = chnl.write_some_at(0, buf, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = chnl.write_some_at(1, buf + 1, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = chnl.write_some_at(2, buf + 2, 1);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 1);

    nbytes = chnl.write_some_at(3, buf + 3, 1);
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);

    REQUIRE(out[0] == 'A');
    REQUIRE(out[1] == 'B');
    REQUIRE(out[2] == 'C');
    REQUIRE(out[3] == 'X');
}

TEST_CASE("omemchannel::write_some() past the end")
{
    std::byte const buf[] = {
        std::byte('A'), std::byte('B'), std::byte('C'), std::byte('D')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out, out + 3);

    auto nbytes = chnl.write_some(buf, 4);
    REQUIRE(nbytes);
    REQUIRE(*nbytes == 3);
    REQUIRE(out[0] == 'A');
    REQUIRE(out[1] == 'B');
    REQUIRE(out[2] == 'C');
    REQUIRE(out[3] == 'X');
}

TEST_CASE("omemchannel::write_some() with an invalid location")
{
    std::byte const buf[] = {std::byte('A'), std::byte('B'), std::byte('C')};
    char out[4];
    std::memset(out, 'X', sizeof out);

    auto chnl = make_omemchannel(out);

    auto nbytes = chnl.write_some_at(4, buf, 3);
    REQUIRE(!nbytes);
    REQUIRE(nbytes.error() == error::end_of_file);
}

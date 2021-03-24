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

#ifndef SK_CIO_MEMCHANNEL_MEMCHANNEL_HXX
#define SK_CIO_MEMCHANNEL_MEMCHANNEL_HXX

#include <sk/cio/memchannel/detail/memchannel_base.hxx>

namespace sk::cio {

    struct memchannel final : detail::memchannel_base {
        using value_type = std::byte;

        memchannel() = default;
        memchannel(memchannel &&) = default;
        memchannel &operator=(memchannel &&) = default;
        memchannel(memchannel const &) = delete;
        memchannel &operator=(memchannel const &) = delete;
        ~memchannel() = default;

        [[nodiscard]] auto open(std::byte *begin, std::byte *end)
            -> expected<void, std::error_code>
        {
            auto ret = _open(begin, end);
            if (ret)
                _read_position = 0;
            return ret;
        }

        [[nodiscard]] auto open(void *begin, void *end)
            -> expected<void, std::error_code>
        {
            return open(static_cast<std::byte *>(begin),
                        static_cast<std::byte *>(end));
        }

        [[nodiscard]] auto open(signed char *begin, signed char *end)
            -> expected<void, std::error_code>
        {
            return open(reinterpret_cast<std::byte *>(begin),
                        reinterpret_cast<std::byte *>(end));
        }

        [[nodiscard]] auto open(unsigned char *begin, unsigned char *end)
            -> expected<void, std::error_code>
        {
            return open(reinterpret_cast<std::byte *>(begin),
                        reinterpret_cast<std::byte *>(end));
        }

        [[nodiscard]] auto open(std::byte *begin, std::size_t n)
            -> expected<void, std::error_code>
        {
            if (n > (std::numeric_limits<std::uintptr_t>::max() -
                     reinterpret_cast<std::uintptr_t>(begin)))
                return make_unexpected(
                    std::make_error_code(std::errc::bad_address));

            return open(begin, begin + n);
        }

        [[nodiscard]] auto open(signed char *begin, std::size_t n)
            -> expected<void, std::error_code>
        {
            return open(reinterpret_cast<std::byte *>(begin), n);
        }

        [[nodiscard]] auto open(unsigned char *begin, std::size_t n)
            -> expected<void, std::error_code>
        {
            return open(reinterpret_cast<std::byte *>(begin), n);
        }

        [[nodiscard]] auto open(void *begin, std::size_t n)
            -> expected<void, std::error_code>
        {
            return open(reinterpret_cast<std::byte *>(begin), n);
        }

        [[nodiscard]] auto
        read_some_at(io_offset_t loc, std::byte *buffer, io_size_t n)
            -> expected<io_size_t, std::error_code>
        {
            return _read_some_at(loc, buffer, n);
        }

        [[nodiscard]] auto read_some(std::byte *buffer, io_size_t n)
            -> expected<io_size_t, std::error_code>
        {
            auto ret = read_some_at(_read_position, buffer, n);
            if (ret)
                _read_position += *ret;
            return ret;
        }

        [[nodiscard]] auto
        async_read_some_at(io_offset_t loc, std::byte *buf, io_size_t n)
            -> task<expected<io_size_t, std::error_code>>
        {
            co_return read_some_at(loc, buf, n);
        }

        [[nodiscard]] auto async_read_some(std::byte *buf, io_size_t n)
            -> task<expected<io_size_t, std::error_code>>
        {
            co_return read_some(buf, n);
        }

        [[nodiscard]] auto
        write_some_at(io_offset_t loc, std::byte const *buf, io_size_t n)
            -> expected<io_size_t, std::error_code>
        {
            return _write_some_at(loc, buf, n);
        }

        [[nodiscard]] auto write_some(std::byte const *buffer, io_size_t n)
            -> expected<io_size_t, std::error_code>
        {
            auto ret = write_some_at(_write_position, buffer, n);
            if (ret)
                _write_position += *ret;
            return ret;
        }

        [[nodiscard]] auto
        async_write_some_at(io_offset_t loc, std::byte const *buf, io_size_t n)
            -> task<expected<io_size_t, std::error_code>>
        {
            co_return write_some_at(loc, buf, n);
        }

        [[nodiscard]] auto async_write_some(std::byte const *buf, io_size_t n)
            -> task<expected<io_size_t, std::error_code>>
        {
            co_return write_some(buf, n);
        }

    private:
        std::size_t _read_position = 0;
        std::size_t _write_position = 0;
    };

} // namespace sk::cio

#endif // SK_CIO_MEMCHANNEL_MEMCHANNEL_HXX

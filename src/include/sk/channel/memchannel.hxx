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

#ifndef SK_CHANNEL_MEMCHANNEL_HXX_INCLUDED
#define SK_CHANNEL_MEMCHANNEL_HXX_INCLUDED

#include <cstddef>
#include <cstring>
#include <ranges>
#include <span>
#include <system_error>

#include <sk/channel/error.hxx>
#include <sk/channel/types.hxx>
#include <sk/check.hxx>
#include <sk/expected.hxx>
#include <sk/task.hxx>

namespace sk {

    namespace detail {

        class memchannel_base {
        public:
            memchannel_base(memchannel_base const &) = delete;
            auto operator=(memchannel_base const &)
                -> memchannel_base & = delete;

        protected:
            using value_type = std::byte;

            memchannel_base() noexcept = default;
            ~memchannel_base() = default;

            memchannel_base(std::span<std::byte> memory_) noexcept
                : memory(memory_)
            {
            }

            memchannel_base(memchannel_base &&) noexcept = default;
            auto operator=(memchannel_base &&) noexcept
                -> memchannel_base & = default;

            [[nodiscard]] auto is_open() const noexcept -> bool
            {
                return memory.data() != nullptr;
            }

            [[nodiscard]] auto async_close() noexcept
                -> task<expected<void, std::error_code>>
            {
                memory = std::span<std::byte>();
                co_return {};
            }

            [[nodiscard]] auto close() noexcept
                -> expected<void, std::error_code>
            {
                memory = std::span<std::byte>();
                return {};
            }

            [[nodiscard]] auto read_some(std::span<value_type> buf) noexcept
                -> expected<io_size_t, std::error_code>
            {
                auto ret = read_some_at(_read_position, buf);
                if (ret)
                    _read_position += *ret;
                return ret;
            }

            [[nodiscard]] auto
            async_read_some(std::span<value_type> buf) noexcept
                -> task<expected<io_size_t, std::error_code>>
            {
                co_return read_some(buf);
            }

            [[nodiscard]] auto read_some_at(io_offset_t loc,
                                            std::span<value_type> buf) noexcept
                -> expected<io_size_t, std::error_code>
            {
                if (loc > memory.size())
                    return make_unexpected(sk::error::end_of_file);

                auto omem = memory.subspan(loc);
                if (buf.size() > omem.size())
                    buf = buf.first(omem.size());

                if (buf.empty())
                    return make_unexpected(sk::error::end_of_file);

                std::memcpy(buf.data(), omem.data(), buf.size());
                return buf.size();
            }

            [[nodiscard]] auto
            write_some_at(io_offset_t loc,
                          std::span<value_type const> buf) noexcept
                -> expected<io_size_t, std::error_code>
            {
                if (loc > memory.size())
                    return make_unexpected(sk::error::end_of_file);

                auto omem = memory.subspan(loc);
                if (buf.size() > omem.size())
                    buf = buf.first(omem.size());

                if (buf.empty())
                    return make_unexpected(sk::error::end_of_file);

                std::memcpy(omem.data(), buf.data(), buf.size());
                return buf.size();
            }

            [[nodiscard]] auto
            write_some(std::span<value_type const> buf) noexcept
                -> expected<io_size_t, std::error_code>
            {
                auto ret = write_some_at(_write_position, buf);
                if (ret)
                    _write_position += *ret;
                return ret;
            }

            [[nodiscard]] auto
            async_write_some(std::span<value_type const> buf) noexcept
                -> task<expected<io_size_t, std::error_code>>
            {
                co_return write_some(buf);
            }

            [[nodiscard]] auto
            async_read_some_at(io_offset_t loc,
                               std::span<value_type> buf) noexcept
                -> task<expected<io_size_t, std::error_code>>
            {
                co_return read_some_at(loc, buf);
            }

            [[nodiscard]] auto
            async_write_some_at(io_offset_t loc,
                                std::span<value_type const> buf) noexcept
                -> task<expected<io_size_t, std::error_code>>
            {
                co_return write_some_at(loc, buf);
            }

        private:
            std::span<std::byte> memory;
            std::size_t _read_position = 0;
            std::size_t _write_position = 0;
        };

    } // namespace detail

    class imemchannel final : protected detail::memchannel_base {
    public:
        using typename detail::memchannel_base::value_type;

        imemchannel(std::span<std::byte const> memory_) noexcept
            : detail::memchannel_base(std::span<std::byte>(
                  const_cast<std::byte *>(memory_.data()), memory_.size()))
        {
        }

        imemchannel(imemchannel &&) noexcept = default;
        auto operator=(imemchannel &&) noexcept -> imemchannel & = default;
        ~imemchannel() = default;

        imemchannel(imemchannel const &) = delete;
        auto operator=(imemchannel const &) -> imemchannel & = delete;

        using detail::memchannel_base::async_close;
        using detail::memchannel_base::async_read_some;
        using detail::memchannel_base::async_read_some_at;
        using detail::memchannel_base::close;
        using detail::memchannel_base::is_open;
        using detail::memchannel_base::read_some;
        using detail::memchannel_base::read_some_at;
    };

    [[nodiscard]] inline auto make_imemchannel(void const *begin,
                                               void const *end) noexcept
    {
        return imemchannel(
            std::span<std::byte const>(static_cast<std::byte const *>(begin),
                                       static_cast<std::byte const *>(end)));
    }

    template <std::ranges::contiguous_range Range>
    [[nodiscard]] auto make_imemchannel(Range &&r) noexcept
    {
        return imemchannel(as_bytes(std::span(r)));
    }

    class omemchannel final : protected detail::memchannel_base {
    public:
        using typename detail::memchannel_base::value_type;

        omemchannel(std::span<value_type> memory_) noexcept
            : detail::memchannel_base(memory_)
        {
        }

        ~omemchannel() = default;

        omemchannel(omemchannel &&other) noexcept = default;
        auto operator=(omemchannel &&) noexcept -> omemchannel & = default;

        omemchannel(omemchannel const &) = delete;
        auto operator=(omemchannel const &) -> omemchannel & = delete;

        using detail::memchannel_base::async_close;
        using detail::memchannel_base::async_write_some;
        using detail::memchannel_base::async_write_some_at;
        using detail::memchannel_base::close;
        using detail::memchannel_base::is_open;
        using detail::memchannel_base::write_some;
        using detail::memchannel_base::write_some_at;
    };

    [[nodiscard]] inline auto make_omemchannel(void *begin, void *end) noexcept
    {
        return omemchannel(std::span<std::byte>(static_cast<std::byte *>(begin),
                                                static_cast<std::byte *>(end)));
    }

    template <std::ranges::contiguous_range Range>
    [[nodiscard]] auto make_omemchannel(Range &&r) noexcept
    {
        return omemchannel(
            as_writable_bytes(std::span(std::forward<Range>(r))));
    }

    class memchannel final : protected detail::memchannel_base {
    public:
        using typename detail::memchannel_base::value_type;

        memchannel(std::span<value_type> memory_) noexcept
            : detail::memchannel_base(memory_)
        {
        }

        memchannel(memchannel &&) noexcept = default;
        auto operator=(memchannel &&) noexcept -> memchannel & = default;
        ~memchannel() = default;

        memchannel(memchannel const &) = delete;
        auto operator=(memchannel const &) -> memchannel & = delete;

        using detail::memchannel_base::async_close;
        using detail::memchannel_base::async_read_some;
        using detail::memchannel_base::async_read_some_at;
        using detail::memchannel_base::async_write_some;
        using detail::memchannel_base::async_write_some_at;
        using detail::memchannel_base::close;
        using detail::memchannel_base::is_open;
        using detail::memchannel_base::read_some;
        using detail::memchannel_base::read_some_at;
        using detail::memchannel_base::write_some;
        using detail::memchannel_base::write_some_at;
    };

    [[nodiscard]] inline auto make_memchannel(void *begin, void *end) noexcept
    {
        return memchannel(std::span<std::byte>(static_cast<std::byte *>(begin),
                                               static_cast<std::byte *>(end)));
    }

    template <std::ranges::contiguous_range Range>
    [[nodiscard]] auto make_memchannel(Range &&r) noexcept
    {
        return memchannel(as_writable_bytes(std::span(std::forward<Range>(r))));
    }

} // namespace sk

#endif // SK_CHANNEL_MEMCHANNEL_HXX_INCLUDED

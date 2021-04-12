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

#ifndef SK_CHANNEL_FILECHANNEL_HXX_INCLUDED
#define SK_CHANNEL_FILECHANNEL_HXX_INCLUDED

#include <cstdint>
#include <filesystem>
#include <system_error>

#include <sk/channel/concepts.hxx>
#include <sk/channel/error.hxx>
#include <sk/channel/types.hxx>
#include <sk/detail/platform.hxx>
#include <sk/flagset.hxx>
#include <sk/task.hxx>

namespace sk {

    struct fileflag_tag {};
    using fileflag = flag<fileflag_tag, std::uint_fast16_t>;

    namespace fileflags {
        // No flags
        static constexpr fileflag none{0u};

        // Open the file for writing.
        static constexpr fileflag write{1u << 0};

        // Open the file for reading.
        static constexpr fileflag read{1u << 1};

        // When opening a file for writing, truncate it.
        static constexpr fileflag trunc{1u << 2};

        // When opening a file for writing, seek to the end.
        static constexpr fileflag append{1u << 3};

        // When opening a file for writing, allowing creating a new file.
        static constexpr fileflag create_new{1u << 4};

        // When opening a file, allowing opening an existing file.
        // This can be specified for input files, but it's implied anyway.
        static constexpr fileflag open_existing{1u << 5};
    } // namespace fileflags

} // namespace sk

#if defined(SK_CIO_PLATFORM_WINDOWS)
#    include <sk/win32/detail/filechannel_base.hxx>

namespace sk::detail {
    using dafilechannel_base = sk::win32::detail::dafilechannel_base;
    using seqfilechannel_base = sk::win32::detail::seqfilechannel_base;
} // namespace sk::detail

#elif defined(SK_CIO_PLATFORM_POSIX)
#    include <sk/posix/detail/filechannel_base.hxx>

namespace sk::detail {
    using dafilechannel_base = sk::posix::detail::dafilechannel_base;
    using seqfilechannel_base = sk::posix::detail::seqfilechannel_base;
} // namespace sk::detail

#else

#    error filechannel is not supported on this platform

#endif

namespace sk {

    /*************************************************************************
     *
     * idafilechannel: a direct access channel that reads from a file.
     */

    class idafilechannel final : protected detail::dafilechannel_base {
    public:
        using dafilechannel_base::value_type;
        using dafilechannel_base::is_open;
        using dafilechannel_base::async_close;
        using dafilechannel_base::close;
        using dafilechannel_base::async_read_some_at;
        using dafilechannel_base::read_some_at;

        /*
         * Create an idafilechannel which is closed.
         */
        idafilechannel() noexcept = default;
        ~idafilechannel() = default;

        idafilechannel(idafilechannel &&) noexcept = default;
        auto operator=(idafilechannel &&) noexcept
            -> idafilechannel & = default;

        idafilechannel(idafilechannel const &) = delete;
        auto operator=(idafilechannel const &) -> idafilechannel & = delete;

        [[nodiscard]] auto
        async_open(std::filesystem::path const &path,
                   fileflag::flagset flags = fileflags::none) noexcept
            -> task<expected<void, std::error_code>>
        {
            if (is_set(flags, fileflags::write))
                co_return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::read;
            co_return co_await this->async_da_open(path, flags);
        }

        [[nodiscard]] auto open(std::filesystem::path const &path,
                                fileflag::flagset flags = fileflags::none) noexcept
            -> expected<void, std::error_code>
        {
            if (is_set(flags, fileflags::write))
                return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::read;
            return this->da_open(path, flags);
        }
    };

    static_assert(idachannel<idafilechannel>);

    /*************************************************************************
     *
     * odafilechannel: a direct access channel that writes to a file.
     */

    class odafilechannel final : protected detail::dafilechannel_base {
    public:
        using dafilechannel_base::value_type;
        using dafilechannel_base::is_open;
        using dafilechannel_base::async_close;
        using dafilechannel_base::close;
        using dafilechannel_base::async_write_some_at;
        using dafilechannel_base::write_some_at;

        /*
         * Create an odafilechannel which is closed.
         */
        odafilechannel() noexcept = default;
        ~odafilechannel() = default;

        odafilechannel(odafilechannel &&) noexcept = default;
        auto operator=(odafilechannel &&) noexcept
            -> odafilechannel & = default;

        odafilechannel(odafilechannel const &) = delete;
        auto operator=(odafilechannel const &) -> odafilechannel & = delete;

        /*
         * Open a file.
         */
        [[nodiscard]] auto
        async_open(std::filesystem::path const &path,
                   fileflag::flagset flags = fileflags::none) noexcept
            -> task<expected<void, std::error_code>>
        {
            if (is_set(flags, fileflags::read))
                co_return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::write;
            co_return co_await this->async_da_open(path, flags);
        }

        [[nodiscard]] auto open(std::filesystem::path const &path,
                                fileflag::flagset flags = fileflags::none) noexcept
            -> expected<void, std::error_code>
        {
            if (is_set(flags, fileflags::read))
                return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::write;
            return this->da_open(path, flags);
        }
    };

    static_assert(odachannel<odafilechannel>);

    /*************************************************************************
     *
     * dafilechannel: a direct access channel that reads and writes a file.
     */

    class dafilechannel final : protected detail::dafilechannel_base {
    public:
        using dafilechannel_base::value_type;
        using dafilechannel_base::is_open;
        using dafilechannel_base::async_close;
        using dafilechannel_base::close;
        using dafilechannel_base::async_read_some_at;
        using dafilechannel_base::read_some_at;
        using dafilechannel_base::async_write_some_at;
        using dafilechannel_base::write_some_at;

        /*
         * Create an dafilechannel which is closed.
         */
        dafilechannel() noexcept = default;
        ~dafilechannel() = default;

        dafilechannel(dafilechannel &&) noexcept = default;
        auto operator=(dafilechannel &&) noexcept -> dafilechannel & = default;

        dafilechannel(dafilechannel const &) = delete;
        auto operator=(dafilechannel const &) -> dafilechannel & = delete;

        /*
         * Open a file.
         */
        [[nodiscard]] auto
        async_open(std::filesystem::path const &path,
                   fileflag::flagset flags = fileflags::none) noexcept
            -> task<expected<void, std::error_code>>
        {
            flags |= fileflags::read | fileflags::write;
            co_return co_await this->async_da_open(path, flags);
        }

        [[nodiscard]] auto open(std::filesystem::path const &path,
                                fileflag::flagset flags = fileflags::none) noexcept
            -> expected<void, std::error_code>
        {
            flags |= fileflags::read | fileflags::write;
            return this->da_open(path, flags);
        }
    };

    static_assert(dachannel<dafilechannel>);

    /*************************************************************************
     *
     * iseqfilechannel: a sequential-access channel to a file.
     *
     */
    class iseqfilechannel final : protected detail::seqfilechannel_base {
    public:
        using seqfilechannel_base::value_type;
        using seqfilechannel_base::is_open;
        using seqfilechannel_base::async_close;
        using seqfilechannel_base::close;
        using seqfilechannel_base::async_read_some;
        using seqfilechannel_base::read_some;

        iseqfilechannel() noexcept = default;
        ~iseqfilechannel() = default;

        auto operator=(iseqfilechannel &&) noexcept
            -> iseqfilechannel & = default;
        iseqfilechannel(iseqfilechannel &&) noexcept = default;

        explicit iseqfilechannel(iseqfilechannel const &) = delete;
        auto operator=(iseqfilechannel const &) -> iseqfilechannel & = delete;

        /*
         * Open a file.
         */
        [[nodiscard]] auto
        async_open(std::filesystem::path const &path,
                   fileflag::flagset flags = fileflags::none) noexcept
            -> task<expected<void, std::error_code>>
        {
            if (is_set(flags, fileflags::write))
                co_return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::read;
            co_return co_await async_seq_open(path, flags);
        }

        [[nodiscard]] auto open(std::filesystem::path const &path,
                                fileflag::flagset flags = fileflags::none) noexcept
            -> expected<void, std::error_code>
        {
            if (is_set(flags, fileflags::write))
                return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::read;
            return seq_open(path, flags);
        }
    };

    static_assert(iseqchannel<iseqfilechannel>);

    /*************************************************************************
     *
     * oseqfilechannel: a sequential channel that writes to a file.
     */

    class oseqfilechannel final : protected detail::seqfilechannel_base {
    public:
        using seqfilechannel_base::value_type;
        using seqfilechannel_base::is_open;
        using seqfilechannel_base::async_close;
        using seqfilechannel_base::close;
        using seqfilechannel_base::async_write_some;
        using seqfilechannel_base::write_some;

        /*
         * Create an oseqfilechannel which is closed.
         */
        oseqfilechannel() noexcept = default;

        oseqfilechannel(oseqfilechannel const &) = delete;
        oseqfilechannel(oseqfilechannel &&) noexcept = default;
        auto operator=(oseqfilechannel const &) -> oseqfilechannel & = delete;
        auto operator=(oseqfilechannel &&) noexcept
            -> oseqfilechannel & = default;
        ~oseqfilechannel() = default;

        /*
         * Open a file.
         */
        [[nodiscard]] auto async_open(std::filesystem::path const &path,
                                      fileflag::flagset flags = fileflags::none) noexcept
            -> task<expected<void, std::error_code>>
        {
            if (is_set(flags, fileflags::read))
                co_return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::write;
            co_return co_await this->async_seq_open(path, flags);
        }

        [[nodiscard]] auto open(std::filesystem::path const &path,
                                fileflag::flagset flags = fileflags::none) noexcept
            -> expected<void, std::error_code>
        {
            if (is_set(flags, fileflags::read))
                return make_unexpected(sk::error::filechannel_invalid_flags);

            flags |= fileflags::write;
            return this->seq_open(path, flags);
        }
    };

    static_assert(oseqchannel<oseqfilechannel>);

    /*************************************************************************
     *
     * seqfilechannel: a direct access channel that writes to a file.
     */

    class seqfilechannel final : protected detail::seqfilechannel_base {
    public:
        using seqfilechannel_base::value_type;
        using seqfilechannel_base::is_open;
        using seqfilechannel_base::async_close;
        using seqfilechannel_base::close;
        using seqfilechannel_base::async_write_some;
        using seqfilechannel_base::write_some;
        using seqfilechannel_base::async_read_some;
        using seqfilechannel_base::read_some;

        /*
         * Create a seqfilechannel which is closed.
         */
        seqfilechannel() noexcept = default;

        seqfilechannel(seqfilechannel const &) = delete;
        seqfilechannel(seqfilechannel &&) noexcept = default;
        auto operator=(seqfilechannel const &) -> seqfilechannel & = delete;
        auto operator=(seqfilechannel &&) noexcept
            -> seqfilechannel & = default;
        ~seqfilechannel() noexcept = default;

        /*
         * Open a file.
         */
        [[nodiscard]] auto
        async_open(std::filesystem::path const &path,
                   fileflag::flagset flags = fileflags::none) noexcept
            -> task<expected<void, std::error_code>>
        {
            flags |= fileflags::read | fileflags::write;
            co_return co_await this->async_seq_open(path, flags);
        }

        [[nodiscard]] auto open(std::filesystem::path const &path,
                                fileflag::flagset flags = fileflags::none) noexcept
            -> expected<void, std::error_code>
        {
            flags |= fileflags::read | fileflags::write;
            return this->seq_open(path, flags);
        }
    };

    static_assert(seqchannel<seqfilechannel>);

} // namespace sk

#endif // SK_CHANNEL_FILECHANNEL_HXX_INCLUDED

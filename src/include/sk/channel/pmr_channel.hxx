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

#ifndef SK_CIO_CHANNEL_PMR_CHANNEL_HXX_INCLUDED
#define SK_CIO_CHANNEL_PMR_CHANNEL_HXX_INCLUDED

namespace sk {

    /*************************************************************************
     *
     * pmr_[io][seq|da]channel: an adapter to create a runtime-polymorphic async
     * stream from a compile-time polymorphic stream.
     *
     */

    template <typename Char>
    struct pmr_channel_base {
        virtual ~pmr_channel_base() = default;

        using value_type = Char;

        virtual auto close() -> std::error_code = 0;
        virtual auto async_close() -> task<std::error_code> = 0;
    };

    template <typename Char>
    struct pmr_oseqchannel : pmr_channel_base<Char> {
        virtual auto
        async_write(sk::pmr_readable_buffer<
                    typename basic_pmr_async_stream<Char>::value_type> &)
            -> task<std::error_code> = 0;
    };

    template <typename Char>
    struct pmr_iseqchannel : pmr_channel_base<Char> {
        virtual auto
        async_read(sk::pmr_writable_buffer<
                   typename basic_pmr_async_stream<Char>::value_type> &)
            -> expected<typename basic_pmr_async_stream<Char>::size_type,
                        std::error_code> = 0;
    };

    template <typename Char>
    struct pmr_seqchannel : pmr_oseqchannel<Char>, pmr_iseqchannel<Char> {
    };

    /*************************************************************************
     *
     * PMR adapters: create runtime-polymorphic wrappers for existing channels.
     *
     */

    /*
     * iseqchannel adapter
     */
    template <iseqchannel Channel>
    struct pmr_iseqchannel_adapter
        : pmr_iseqchannel<typename Channel::value_type> {
        Channel readable;

        explicit basic_pmr_async_readable_model(Readable &&readable_)
            : readable(std::move(readable_))
        {
        }

        virtual auto
        async_read(sk::pmr_writable_buffer<typename Readable::value_type> &buf)
            -> expected<std::size_t, sk::error>
        {

            return readable.async_read(buf);
        }
    };

    template <typename Char>
    struct basic_pmr_async_readable_adapter : basic_pmr_async_readable<Char> {
        std::unique_ptr<basic_pmr_async_readable_model_base<Char>>
            model_base_pointer;

        template <async_readable Readable>
        basic_pmr_async_readable_adapter(Readable &&readable)
            : model_base_pointer(
                  std::make_unique<basic_pmr_async_readable_model<Readable>>(
                      std::move(readable)))
        {
        }

        virtual auto async_read(sk::pmr_writable_buffer<Char> &buf)
            -> expected<std::size_t, sk::error>
        {

            return model_base_pointer->async_read(buf);
        }
    };

    template <async_readable Stream>
    auto make_pmr_async_readable(Stream &&strm)
    {
        return basic_pmr_async_readable_adapter<typename Stream::value_type>(
            std::move(strm));
    }

    /*
     * async_writable adapter.
     */
    template <typename Char>
    struct basic_pmr_async_writable_adapter_base {
        virtual auto async_read(sk::pmr_readable_buffer<Char> &buf)
            -> expected<std::size_t, sk::error> = 0;
    };

    template <async_writable_of<char> Writable, typename Char>
    struct basic_pmr_async_writable_adapter {
        Writable writable;

        explicit basic_pmr_async_writable_adapter(Writable &&writable_)
            : writable(std::move(writable_))
        {
        }

        virtual auto async_read(sk::pmr_readable_buffer<Char> &buf)
            -> expected<typename Writable::size_type, sk::error>
        {

            return writable.async_read(buf);
        }
    };

    template <typename Stream>
    using pmr_async_writable_adapter =
        basic_pmr_async_writable_adapter<Stream, char>;

    template <typename Stream>
    using wpmr_async_writable_adapter =
        basic_pmr_async_writable_adapter<Stream, wchar_t>;

    template <typename Stream>
    using u8pmr_async_writable_adapter =
        basic_pmr_async_writable_adapter<Stream, char8_t>;

    template <typename Stream>
    using u16pmr_async_writable_adapter =
        basic_pmr_async_writable_adapter<Stream, char16_t>;

    template <typename Stream>
    using u32pmr_async_writable_adapter =
        basic_pmr_async_writable_adapter<Stream, char32_t>;

    template <async_writable Stream>
    auto make_pmr_async_writable(Stream &&strm)
    {
        return basic_pmr_async_writable_adapter<Stream,
                                                typename Stream::value_type>(
            strm);
    }

} // namespace sk

#endif // SK_CIO_CHANNEL_PMR_CHANNEL_HXX_INCLUDED
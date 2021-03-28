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

/*
 * Concepts of buffers.
 */

#ifndef SK_BUFFER_PMR_BUFFER_HXX_INCLUDED
#define SK_BUFFER_PMR_BUFFER_HXX_INCLUDED

#include <sk/buffer.hxx>

namespace sk {

    /*************************************************************************
     *
     * pmr_buffer: buffer adapter for runtime polymorphism.  This allows a
     * buffer to be converted from compile-time polymorphism to runtime
     * polymorphism through type erasure.  Like any form of runtime
     * polymorphism, overhead is incurred for virtual function calls.
     *
     * A pmr_buffer still conforms to the buffer concept, so it can be passed
     * back to compile-time polymorphic users.  However, the only range type
     * supported for reading and writing the buffer is
     * std::span<value_type, std::dynamic_extent>.
     */

    /*
     * pmr_basic_buffer: basic types and virtual dtor for the pmr buffers.
     */
    template <typename Char> struct pmr_basic_buffer {
        virtual ~pmr_basic_buffer() = default;

        typedef std::size_t size_type;
        typedef Char value_type;
        typedef std::add_const_t<value_type> const_value_type;
    };

    /*
     * pmr_readable_buffer: interface for pmr readable_buffer<>.
     */
    template <typename Char>
    struct pmr_readable_buffer : pmr_basic_buffer<Char> {
        virtual auto
        read(std::span<typename pmr_basic_buffer<Char>::value_type> const &) ->
            typename pmr_basic_buffer<Char>::size_type = 0;

        virtual auto readable_ranges() -> std::vector<
            std::span<typename pmr_basic_buffer<Char>::const_value_type>> = 0;

        virtual auto discard(typename pmr_basic_buffer<Char>::size_type) ->
            typename pmr_basic_buffer<Char>::size_type = 0;
    };

    static_assert(readable_buffer_of<pmr_readable_buffer<char>, char>);

    /*
     * pmr_writable_buffer: interface for pmr writable_buffer<>.
     */
    template <typename Char>
    struct pmr_writable_buffer : pmr_basic_buffer<Char> {
        virtual auto write(std::span<std::add_const_t<Char>> const &) ->
            typename pmr_basic_buffer<Char>::size_type = 0;

        virtual auto writable_ranges() -> std::vector<
            std::span<typename pmr_basic_buffer<Char>::value_type>> = 0;

        virtual auto commit(typename pmr_basic_buffer<Char>::size_type) ->
            typename pmr_basic_buffer<Char>::size_type = 0;
    };

    static_assert(writable_buffer_of<pmr_writable_buffer<char>, char>);

    /*
     * pmr_buffer: interface for pmr buffer<>.
     */
    template <typename Char>
    struct pmr_buffer : pmr_readable_buffer<Char>, pmr_writable_buffer<Char> {};

    /*************************************************************************
     *
     * PMR buffer adapters.  These are thing wrappers over an existing buffer
     * which turn it into a pmr_buffer.  The wrapped buffer is held by
     * reference, so it should not be destroyed before the buffer adapter.
     */

    /*
     * PMR adapter for readable_buffer.
     */
    template <readable_buffer Buffer>
    struct pmr_readable_buffer_adapter
        : pmr_readable_buffer<buffer_value_t<Buffer>> {
        Buffer &buffer_base;

        explicit pmr_readable_buffer_adapter(Buffer &buffer_base_)
            : buffer_base(buffer_base_) {}

        auto read(std::span<buffer_value_t<Buffer>> const &buf) ->
            typename pmr_readable_buffer<buffer_value_t<Buffer>>::size_type
            final {
            return buffer_base.read(buf);
        }

        auto readable_ranges()
            -> std::vector<std::span<typename pmr_readable_buffer<
                buffer_value_t<Buffer>>::const_value_type>> final {
            return buffer_base.readable_ranges();
        }

        auto discard(
            typename pmr_readable_buffer<buffer_value_t<Buffer>>::size_type n)
            -> typename pmr_readable_buffer<buffer_value_t<Buffer>>::size_type
            final {
            return buffer_base.discard(n);
        }
    };

    /*
     * PMR adapter for writable_buffer.
     */
    template <writable_buffer Buffer>
    struct pmr_writable_buffer_adapter
        : pmr_writable_buffer<buffer_value_t<Buffer>> {
        Buffer &buffer_base;

        explicit pmr_writable_buffer_adapter(Buffer &buffer_base_)
            : buffer_base(buffer_base_) {}

        auto
        write(std::span<std::add_const_t<buffer_value_t<Buffer>>> const &buf) ->
            typename pmr_writable_buffer<buffer_value_t<Buffer>>::size_type
            final {
            return buffer_base.write(buf);
        }

        auto writable_ranges()
            -> std::vector<std::span<typename pmr_writable_buffer<
                buffer_value_t<Buffer>>::value_type>> final {
            return buffer_base.writable_ranges();
        }

        auto commit(
            typename pmr_writable_buffer<buffer_value_t<Buffer>>::size_type n)
            -> typename pmr_writable_buffer<buffer_value_t<Buffer>>::size_type
            final {
            return buffer_base.commit(n);
        }
    };

    /*
     * PMR adapter for buffer<>.  This is just a combined readable and writable
     * adapter, so a minor storage overhead is incurred for storing two
     * references to the wrapped buffer.
     */
    template <buffer Buffer>
    struct pmr_buffer_adapter : pmr_readable_buffer_adapter<Buffer>,
                                pmr_writable_buffer_adapter<Buffer> {

        pmr_buffer_adapter(Buffer &buffer_base_)
            : pmr_readable_buffer_adapter<Buffer>(buffer_base_),
              pmr_writable_buffer_adapter<Buffer>(buffer_base_) {}
    };

    /*
     * Utility function to create a pmr_buffer_adapter.
     */

    template <buffer Buffer,
              typename std::enable_if_t<buffer<Buffer>, bool> = true>
    auto make_pmr_buffer_adapter(Buffer &buf) {
        return pmr_buffer_adapter<Buffer>(buf);
    }

    template <readable_buffer Buffer,
              typename std::enable_if_t<!writable_buffer<Buffer>, bool> = true>
    auto make_pmr_buffer_adapter(Buffer &buf) {
        return pmr_readable_buffer_adapter<Buffer>(buf);
    }

    template <writable_buffer Buffer,
              typename std::enable_if_t<!readable_buffer<Buffer>, bool> = true>
    auto make_pmr_buffer_adapter(Buffer &buf) {
        return pmr_writable_buffer_adapter<Buffer>(buf);
    }

} // namespace sk

#endif // SK_BUFFER_PMR_BUFFER_HXX_INCLUDED

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

#ifndef SK_CIO_ERROR_HXX_INCLUDED
#define SK_CIO_ERROR_HXX_INCLUDED

#include <string>
#include <system_error>

namespace sk::cio {

    /*************************************************************************
     *
     * Channel error codes.
     * 
     */

    enum struct error : int {
        // Operation succeeded
        no_error = 0,

        // End of file reached.
        end_of_file = 1,

        // The buffer passed to a read operation has no space to read into.
        no_space_in_buffer = 2,

        // The buffer passed to a write operation has no data in it.
        no_data_in_buffer = 3,

        // The flags passed to filechannel's open() were not valid.
        filechannel_invalid_flags = 4,

        // Attempt to open a channel which is already open.
        channel_already_open = 5,

        // Attempt to use a channel which is not open.
        channel_not_open = 6,
    };

    namespace detail {

        struct cio_errc_category : std::error_category {
            auto name() const noexcept -> char const * final;
            auto message(int c) const -> std::string final;
        };

    } // namespace detail

    auto cio_errc_category() -> detail::cio_errc_category const &;
    std::error_code make_error_code(error e);

} // namespace sk::cio

template <> struct std::is_error_code_enum<sk::cio::error> : true_type {};

#endif // SK_CIO_ERROR_HXX_INCLUDED

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

#ifndef SK_RADIX_HXX_INCLUDED
#define SK_RADIX_HXX_INCLUDED

#include <climits>
#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

#include <fmt/core.h>

namespace sk {

    template <typename T = std::uint8_t>
    struct bitstring {
        static constexpr unsigned max_value_len =
            std::numeric_limits<T>::digits;
        static constexpr T one = T(1) << (std::numeric_limits<T>::digits - 1);

        auto max_len() const -> std::size_t
        {
            return values.size() * std::numeric_limits<T>::digits;
        }

        std::span<T> values;
        unsigned len = 0;

        bitstring() = default;

        bitstring(bitstring const &other) : values(other.values), len(other.len)
        {
        }
        bitstring(bitstring &&other) : values(other.values), len(other.len) {}

        explicit bitstring(std::span<T> values_) : values(values_) {}

        bitstring(std::span<T> values_, bitstring const &other)
            : values(values_)
        {
            len = other.len;
            assert(len <= (values_.size() * max_value_len));
            std::memcpy(
                values.data(), other.values.data(), other.values.size() * sizeof(T));
        }

        bitstring(std::span<T> values_, std::string const &v) : values(values_)
        {
            for (auto &&b : v)
                append(b == '1' ? 1 : 0);
        }

        bitstring(std::span<T> values_, char const *s)
            : bitstring(values_, std::string(s))
        {
        }

        explicit bitstring(std::span<T> values_,
                           std::byte const *bstart,
                           std::byte const *bend)
            : values(values_)
        {
            len = std::distance(bstart, bend) * 8;

            assert(values.size_bytes() >=
                   static_cast<std::size_t>(bend - bstart));

            if (sizeof(T) == 1 || (std::endian::native == std::endian::big)) {
                std::memcpy(values.data(), bstart, bend - bstart);
            } else {
                auto value = values.begin();
                *value = 0;
                unsigned shift = max_value_len - 8;

                while (bstart < bend) {
                    *value |= static_cast<T>(*bstart) << shift;
                    if (shift == 0) {
                        value++;
                        *value = 0;
                        shift = max_value_len - 8;
                    } else {
                        shift -= 8;
                    }
                    bstart++;
                }
            }
        }

        auto operator=(std::string const &v) -> bitstring &
        {
            reset();
            for (auto &&b : v) {
                if (b == '\'')
                    continue;

                append(b == '1' ? 1 : 0);
            }
            return *this;
        }

        auto operator=(bitstring const &other) -> bitstring &
        {
            if (&other != this) {
                assert(other.len <= max_len());
                len = other.len;
                std::ranges::copy(other.values, values.begin());
            }

            return *this;
        }

        auto operator=(bitstring &&other) -> bitstring &
        {
            if (&other != this) {
                values = other.values;
                len = other.len;
            }

            return *this;
        }

        auto value_for_bit(unsigned bit) const -> T const &
        {
            auto n = bit / max_value_len;
            assert(n < values.size());
            return values[n];
        }

        auto value_for_bit(unsigned bit) -> T &
        {
            return const_cast<T &>(
                static_cast<bitstring const *>(this)->value_for_bit(bit));
        }

        auto bit_for_bit(unsigned bit) const -> unsigned
        {
            auto n = bit % max_value_len;
            return n;
        }

        auto mask_for_bit(unsigned bit) const -> T
        {
            return one >> bit_for_bit(bit);
        }

        void set(unsigned bit)
        {
            value_for_bit(bit) |= mask_for_bit(bit_for_bit(bit));
            assert(test(bit));
        }

        void clr(unsigned bit)
        {
            value_for_bit(bit) &= ~mask_for_bit(bit_for_bit(bit));
            assert(!test(bit));
        }

        auto test(unsigned bit) const -> bool
        {
            assert(bit <= len);
            return (value_for_bit(bit) & mask_for_bit(bit)) ? 1 : 0;
        }

        void append(bool bit)
        {
            assert(len + 1 <= max_len());

            if (bit)
                set(len);
            else
                clr(len);

            ++len;
        }

        auto str() const -> std::string
        {
            std::string r;

            for (unsigned i = 0; i < len; ++i)
                r += test(i) ? '1' : '0';

            return r;
        }

        auto reset() -> void
        {
            len = 0;
        }

        auto operator[](unsigned bit) const -> bool
        {
            return test(bit);
        }

        auto size() -> unsigned
        {
            return len;
        }

        void append(bitstring const &other)
        {
            assert(len + other.len < max_len());

            unsigned n = other.len;
            auto *this_value = &value_for_bit(len + 1);
            auto *other_value = &other.value_for_bit(0);
            unsigned offs = len % max_value_len;

            while (n) {
                unsigned can = std::min(n, max_value_len);
                auto next_value = this_value + 1;

                *this_value &= ~(T(~0) >> offs);
                *this_value |= *other_value >> offs;

                if ((can + offs) > max_value_len)
                    *next_value = (*other_value << (max_value_len - offs));

                ++this_value;
                ++other_value;
                len += can;
                n -= can;
            }
        }

        auto operator<<=(unsigned bits) -> bitstring &
        {
            assert(len >= bits);

            int offs = bits / max_value_len;
            auto *shift_value = &values[0] + offs;

            int shift = bits % max_value_len;
            unsigned bits_left = len;

            for (unsigned i = 0; i < values.size(); ++i) {
                if (!bits_left)
                    break;

                if (shift_value < (values.data() + values.size()))
                    values[i] = *shift_value << shift;

                if (shift_value + 1 < (values.data() + values.size()))
                    values[i] |=
                        T(*(shift_value + 1)) >> (max_value_len - shift);

                shift_value++;
                bits_left -= max_value_len;
            }

            len -= bits;
            return *this;
        }
    };

    template <std::unsigned_integral T>
    inline auto common_prefix(bitstring<T> const &a, bitstring<T> const &b)
    {
        bitstring<T> ret(a);

        unsigned minlen = std::min(a.len, b.len);
        unsigned bits_left = minlen;

        for (unsigned i = 0;; ++i) {
            if (bits_left > ret.max_value_len && (a.values[i] == b.values[i])) {
                bits_left -= ret.max_value_len;
                continue;
            }

            unsigned len = ret.bit_for_bit(std::min(a.len, b.len));
            if (len == 0)
                len = ret.max_value_len;

            T mask = T(~0) << (ret.max_value_len - len);

            unsigned bits = std::countl_zero(
                T(T(a.values[i] & mask) ^ T(b.values[i] & mask)));
            if (bits > len)
                bits = len;

            ret.len = i * (sizeof(T) * CHAR_BIT) + std::min(ret.len, bits);
            break;
        }

        return ret;
    }

    template <std::unsigned_integral T>
    inline auto operator+(bitstring<T> const &a, bitstring<T> const &b)
    {
        auto r(a);
        r.append(b);
        return r;
    }

    enum struct radix_op : int {
        insert,
        find,
        remove,
    };

    template <typename T>
    struct radix_node {
        struct deleter {
            void operator() (radix_node *n) {
                if (!n)
                    return;
                n->~radix_node();
                std::free(n);
            }
        };

        using node_ptr = std::unique_ptr<radix_node, deleter>;
        using pack_type = std::uint8_t;
        using string_type = bitstring<pack_type>;

        radix_node()
            : istring(key_data)
        {
        }

        radix_node(radix_node const &) = delete;
        radix_node(radix_node &&other) : value(std::move(other.value)),
              left(std::move(other.left)), right(std::move(other.right)) {}

        auto operator=(radix_node const &) = delete;
        auto operator=(radix_node &&) = delete;

        // radix_node(string_type const &s) : istring(s) {}

        auto make_node(string_type const &ir) -> node_ptr
        {
            void *data = std::malloc(sizeof(radix_node) +
                                     (ir.values.size() * sizeof(pack_type)));

            radix_node *n = new (data) radix_node();
            n->istring =
                string_type(std::span(&n->key_data[0], ir.values.size()), ir);

            assert(n == data);
            return node_ptr(n);
        }

        auto make_node(node_ptr &&np, string_type const &ira, string_type const &irb) -> node_ptr
        {
            void *data = std::malloc(sizeof(radix_node) +
                                     (ira.values.size() * sizeof(pack_type))
                                     + (irb.values.size() * sizeof(pack_type)));

            radix_node *n = new (data) radix_node(std::move(*np));
            n->istring =
                string_type(std::span(&n->key_data[0], ira.values.size() + irb.values.size()), ira);
            n->istring.append(irb);

            assert(n == data);
            return node_ptr(n);
        }

        auto add_node(node_ptr &&n) -> node_ptr &
        {
            auto &branch = n->istring[0] ? right : left;
            assert(branch.get() == nullptr);
            branch = std::move(n);
            return branch;
        }

        std::optional<T> value;
        string_type istring;
        node_ptr left, right;
        pack_type key_data[1];

        auto find(string_type ir, radix_op op) -> radix_node<T> *
        {
            if (ir.size() == 0) {
                if (op == radix_op::remove)
                    value.reset();

                return this;
            }

            auto &e = ir[0] ? right : left;

            if (!e) {
                if (op != radix_op::insert)
                    return nullptr;

                auto &new_edge = add_node(make_node(ir));
                return new_edge.get();
            }

            auto pfx = common_prefix(ir, e->istring);

            auto matchlen = pfx.size();

            assert(matchlen > 0);

            if (matchlen == e->istring.size()) {
                ir <<= pfx.size();
                auto ret = e->find(ir, op);
                if (op != radix_op::remove)
                    return ret;

                if (!e->value) {
                    if (!e->left && !e->right) {
                        e.reset();
                        return this;
                    }

                    if (!(e->left && e->right)) {
                        auto &steal = e->left ? e->left : e->right;
                        e = make_node(std::move(steal), e->istring, steal->istring);
                        return this;
                    }
                }

                return ret;
            }

            if (op != radix_op::insert)
                return nullptr;

            auto new_node = make_node(pfx);
            e->istring <<= matchlen;

            new_node->add_node(std::move(e));
            e = std::move(new_node);

            ir <<= pfx.size();
            return e->find(ir, op);
        }
    };

    template <typename Char, typename T>
    struct radix_tree {
        radix_node<T> root;

        template <std::ranges::contiguous_range Range>
        auto insert(Range &&r, T const &value) -> bool
        {
            auto dstart =
                reinterpret_cast<std::byte const *>(std::ranges::data(r));
            auto dend = dstart + std::ranges::size(r) *
                                     sizeof(std::ranges::range_value_t<Range>);
            std::array<typename radix_node<T>::pack_type, 8> data;
            //std::vector<typename radix_node<T>::pack_type> data(
            //    std::distance(dstart, dend));
            typename radix_node<T>::string_type bs(data, dstart, dend);

            auto *node = root.find(bs, radix_op::insert);
            if (node->value)
                return false;

            node->value.emplace(value);
            return true;
        }

        template <std::ranges::contiguous_range Range>
        auto remove(Range &&r) -> bool
        {
            auto dstart =
                reinterpret_cast<std::byte const *>(std::ranges::data(r));
            auto dend = dstart + std::ranges::size(r) *
                                     sizeof(std::ranges::range_value_t<Range>);
            std::vector<typename radix_node<T>::pack_type> data(
                std::distance(dstart, dend));
            typename radix_node<T>::string_type bs(data, dstart, dend);

            auto *node = root.find(bs, radix_op::remove);

            if (!node)
                return false;

            return true;
        }

        template <std::ranges::contiguous_range Range>
        auto find(Range &&r) -> T *
        {
            auto dstart =
                reinterpret_cast<std::byte const *>(std::ranges::data(r));
            auto dend = dstart + std::ranges::size(r) *
                                     sizeof(std::ranges::range_value_t<Range>);
            std::vector<typename radix_node<T>::pack_type> data(
                std::distance(dstart, dend));
            typename radix_node<T>::string_type bs(data, dstart, dend);

            auto node = root.find(bs, radix_op::find);

            if (!node)
                return nullptr;

            if (!node->value)
                return nullptr;

            return &*node->value;
        }
    };

} // namespace sk

#endif // SK_RADIX_HXX_INCLUDED

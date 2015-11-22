#ifndef QBB_UTIL_LRU_CACHE_HPP
#define QBB_UTIL_LRU_CACHE_HPP

// This code is based on the LRU implementation of Tim Day (URL:
// http://timday.bitbucket.org/lru.html).
// The copyright notice of the original code is reproduced below.

// Copyright (c) 2010-2011, Tim Day <timday@timday.com>
//
// Permission to use, copy, modify, and/or distribute this software for any
//         purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//         ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//        OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <qbb/util/assert.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/set_of.hpp>

#include <functional>
#include <utility>

namespace qbb
{
namespace util
{

// Class providing fixed-size (by number of records)
// LRU-replacement cache of a function with signature
// V f(K).
template <typename K, typename T>
class lru_cache
{
public:
    using key_type = K;
    using value_type = T;

    using container_type =
        boost::bimaps::bimap<boost::bimaps::set_of<key_type>,
                             boost::bimaps::list_of<value_type>>;

    lru_cache(std::function<value_type(const key_type&)> fn_, std::size_t capacity_)
    : fn_(std::move(fn_)), capacity_(std::move(capacity_))
    {
        QBB_ASSERT(capacity_ != 0, "Cache capacity needs to be non-zero.");
    }

    value_type operator()(const key_type& k) const
    {
        const auto it = container_.left.find(k);

        if (it == container_.left.end())
        {
            const value_type v = fn_(k);
            insert(k, v);

            return v;
        }
        else
        {
            container_.right.relocate(container_.right.end(), container_.project_right(it));

            return it->second;
        }
    }

private:
    void insert(const key_type& k, const value_type& v) const
    {
        QBB_ASSERT(container_.size() <= capacity_, "Cache entry count exceeds the capacity.");

        if (container_.size() == capacity_)
        {
            container_.right.erase(container_.right.begin());
        }

        container_.insert(typename container_type::value_type(k, v));
    }

    std::function<value_type(const key_type&)> fn_;
    std::size_t capacity_;
    mutable container_type container_;
};
}
}

#endif

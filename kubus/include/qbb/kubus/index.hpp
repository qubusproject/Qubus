#ifndef KUBUS_INDEX_HPP
#define KUBUS_INDEX_HPP

#include <qbb/util/handle.hpp>

#include <cstddef>
#include <memory>
#include <ostream>
#include <array>
#include <utility>

namespace qbb
{
namespace kubus
{

class index
{
public:
    index() : info_(std::make_shared<index_info>())
    {
    }

    explicit index(const char* debug_name_) : info_(std::make_shared<index_info>(debug_name_))
    {
    }

    qbb::util::handle id() const
    {
        return info_->id();
    }

    const char* debug_name() const
    {
        return info_->debug_name();
    }

private:
    class index_info
    {
    public:
        index_info() : debug_name_(nullptr)
        {
        }

        explicit index_info(const char* debug_name_) : debug_name_(debug_name_)
        {
        }

        qbb::util::handle id() const
        {
            return qbb::util::handle_from_ptr(this);
        }

        const char* debug_name() const
        {
            return debug_name_;
        }

    private:
        const char* debug_name_;
    };

    std::shared_ptr<index_info> info_;
};

inline bool operator==(const index& lhs, const index& rhs)
{
    return lhs.id() == rhs.id();
}

inline bool operator!=(const index& lhs, const index& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const index& idx)
{
    return os << "index( " << idx.debug_name() << " )";
}

inline qbb::util::handle id(const index& value)
{
    return value.id();
}

template <long int Rank>
class multi_index
{
public:
    static_assert(Rank >= 0, "Rank must be non-negative.");

    multi_index() : info_(std::make_shared<multi_index_info>())
    {
    }

    multi_index(std::array<index, Rank> element_indices_)
    : info_(std::make_shared<multi_index_info>(std::move(element_indices_)))
    {
    }

    const index& operator[](long int pos) const
    {
        return (*info_)[pos];
    }
  
    long int rank() const
    {
        return Rank;
    }

    qbb::util::handle id() const
    {
        return qbb::util::handle_from_ptr(this);
    }

    const char* debug_name() const
    {
        return info_->debug_name();
    }

private:
    class multi_index_info
    {
    public:
        multi_index_info() : debug_name_(nullptr)
        {
        }

        explicit multi_index_info(std::array<index, Rank> element_indices_)
        : debug_name_(nullptr), element_indices_(std::move(element_indices_))
        {
        }

        const index& operator[](long int pos) const
        {
            return element_indices_[pos];
        }

        const char* debug_name() const
        {
            return debug_name_;
        }

    private:
        const char* debug_name_;
        std::array<index, Rank> element_indices_;
    };

    std::shared_ptr<multi_index_info> info_;
};

template <long int LHSRank, long int RHSRank>
inline bool operator==(const multi_index<LHSRank>& lhs, const multi_index<RHSRank>& rhs)
{
    return lhs.id() == rhs.id();
}

template <long int LHSRank, long int RHSRank>
inline bool operator!=(const multi_index<LHSRank>& lhs, const multi_index<RHSRank>& rhs)
{
    return !(lhs == rhs);
}

template <long int Rank>
inline std::ostream& operator<<(std::ostream& os, const multi_index<Rank>& idx)
{
    return os << "multi_index( " << idx.debug_name() << " )";
}

template <long int Rank>
inline qbb::util::handle id(const multi_index<Rank>& value)
{
    return value.id();
}

}
}

#endif
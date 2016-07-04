#ifndef QUBUS_INDEX_HPP
#define QUBUS_INDEX_HPP

#include <hpx/include/components.hpp>

#include <qbb/qubus/hpx_utils.hpp>

#include <cstddef>
#include <memory>
#include <ostream>
#include <array>
#include <utility>

namespace qbb
{
namespace qubus
{

class id_type_server : public hpx::components::component_base<id_type_server>
{
};

class id_type : public hpx::components::client_base<id_type, id_type_server>
{
public:
    using base_type = hpx::components::client_base<id_type, id_type_server>;

    id_type() = default;
    id_type(hpx::future<hpx::id_type>&& id);
};

class index
{
public:
    index() : info_(std::make_shared<index_info>()), id_(new_here<id_type_server>())
    {
    }

    explicit index(const char* debug_name_) : info_(std::make_shared<index_info>(debug_name_)), id_(new_here<id_type_server>())
    {
    }

    hpx::naming::gid_type id() const
    {
        return id_.get_id().get_gid();
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

        const char* debug_name() const
        {
            return debug_name_;
        }

    private:
        const char* debug_name_;
    };

    std::shared_ptr<index_info> info_;
    id_type id_;
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

inline hpx::naming::gid_type id(const index& value)
{
    return value.id();
}

template <long int Rank>
class multi_index
{
public:
    static_assert(Rank >= 0, "Rank must be non-negative.");

    multi_index() : info_(std::make_shared<multi_index_info>()), id_(new_here<id_type_server>())
    {
    }

    explicit multi_index(const char* debug_name_)
    : info_(std::make_shared<multi_index_info>(debug_name_)), id_(new_here<id_type_server>())
    {
    }

    multi_index(std::array<index, Rank> element_indices_)
    : info_(std::make_shared<multi_index_info>(std::move(element_indices_))), id_(new_here<id_type_server>())
    {
    }

    multi_index(std::array<index, Rank> element_indices_, const char* debug_name_)
    : info_(std::make_shared<multi_index_info>(std::move(element_indices_), debug_name_)), id_(new_here<id_type_server>())
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

    hpx::naming::gid_type id() const
    {
        return id_.get_id().get_gid();
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

        explicit multi_index_info(const char* debug_name_) : debug_name_(debug_name_)
        {
        }

        explicit multi_index_info(std::array<index, Rank> element_indices_)
        : element_indices_(std::move(element_indices_)), debug_name_(nullptr)
        {
        }

        explicit multi_index_info(std::array<index, Rank> element_indices_, const char* debug_name_)
        : element_indices_(std::move(element_indices_)), debug_name_(debug_name_)
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
        std::array<index, Rank> element_indices_;
        const char* debug_name_;
    };

    std::shared_ptr<multi_index_info> info_;
    id_type id_;
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
inline hpx::naming::gid_type id(const multi_index<Rank>& value)
{
    return value.id();
}
}
}

#endif
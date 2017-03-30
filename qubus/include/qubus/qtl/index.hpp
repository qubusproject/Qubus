#ifndef QUBUS_INDEX_HPP
#define QUBUS_INDEX_HPP

#include <qubus/IR/variable_declaration.hpp>

#include <boost/hana/fold.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <type_traits>
#include <utility>

namespace qubus
{
namespace qtl
{
namespace ast
{

template <typename T>
struct is_index : std::false_type
{
};

class index
{
public:
    index() : info_(std::make_shared<index_info>())
    {
    }

    explicit index(const char* debug_name_) : info_(std::make_shared<index_info>(debug_name_))
    {
    }

    const variable_declaration& var() const
    {
        return info_->var();
    }

    const char* debug_name() const
    {
        return info_->debug_name();
    }

private:
    class index_info
    {
    public:
        index_info() : var_(types::index{}), debug_name_(nullptr)
        {
        }

        explicit index_info(const char* debug_name_)
        : var_(types::index{}), debug_name_(debug_name_)
        {
        }

        const variable_declaration& var() const
        {
            return var_;
        }

        const char* debug_name() const
        {
            return debug_name_;
        }

    private:
        variable_declaration var_;
        const char* debug_name_;
    };

    std::shared_ptr<index_info> info_;
};

inline bool operator==(const index& lhs, const index& rhs)
{
    return lhs.var() == rhs.var();
}

inline bool operator!=(const index& lhs, const index& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const index& idx)
{
    return os << "index( " << idx.debug_name() << " )";
}

template <>
struct is_index<index> : std::true_type
{
};

template <long int Rank>
class multi_index
{
public:
    static_assert(Rank >= 0, "Rank must be non-negative.");

    multi_index() : info_(std::make_shared<multi_index_info>())
    {
    }

    explicit multi_index(const char* debug_name_)
    : info_(std::make_shared<multi_index_info>(debug_name_))
    {
    }

    multi_index(std::array<index, Rank> element_indices_)
    : info_(std::make_shared<multi_index_info>(std::move(element_indices_)))
    {
    }

    multi_index(std::array<index, Rank> element_indices_, const char* debug_name_)
    : info_(std::make_shared<multi_index_info>(std::move(element_indices_), debug_name_))
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

    const variable_declaration& var() const
    {
        return info_->var();
    }

    const char* debug_name() const
    {
        return info_->debug_name();
    }

private:
    class multi_index_info
    {
    public:
        multi_index_info() : var_(types::multi_index(Rank)), debug_name_(nullptr)
        {
        }

        explicit multi_index_info(const char* debug_name_)
        : var_(types::multi_index(Rank)), debug_name_(debug_name_)
        {
        }

        explicit multi_index_info(std::array<index, Rank> element_indices_)
        : var_(types::multi_index(Rank)),
          element_indices_(std::move(element_indices_)),
          debug_name_(nullptr)
        {
        }

        explicit multi_index_info(std::array<index, Rank> element_indices_, const char* debug_name_)
        : var_(types::multi_index(Rank)),
          element_indices_(std::move(element_indices_)),
          debug_name_(debug_name_)
        {
        }

        const variable_declaration& var() const
        {
            return var_;
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
        variable_declaration var_;
        std::array<index, Rank> element_indices_;
        const char* debug_name_;
    };

    std::shared_ptr<multi_index_info> info_;
};

template <long int LHSRank, long int RHSRank>
inline bool operator==(const multi_index<LHSRank>& lhs, const multi_index<RHSRank>& rhs)
{
    return lhs.var() == rhs.var();
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
struct is_index<multi_index<Rank>> : std::true_type
{
};

namespace detail
{
struct fulfills_index_concept_t
{
    template <typename T>
    constexpr bool operator()(T type) const
    {
        return is_index<typename decltype(type)::type>::value;
    }
};

constexpr auto fulfills_index_concept = fulfills_index_concept_t{};
}

template <typename... Indices>
constexpr bool are_all_indices()
{
    constexpr auto are_indices = boost::hana::transform(
        boost::hana::make_tuple(boost::hana::type_c<Indices>...), detail::fulfills_index_concept);

    constexpr bool is_valid = boost::hana::fold(are_indices, true, std::logical_and<>());

    return is_valid;
}
}

using index = ast::index;

template <long int Rank>
using multi_index = ast::multi_index<Rank>;
}
}

#endif
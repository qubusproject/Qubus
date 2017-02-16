#ifndef QUBUS_ARRAY_SLICE_EXPR_HPP
#define QUBUS_ARRAY_SLICE_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/access_qualifier.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <vector>

namespace qubus
{

class array_slice_expr : public access_qualifier_base<array_slice_expr>
{
public:
    array_slice_expr() = default;
    array_slice_expr(std::unique_ptr<access_expr> array_,
                     std::vector<std::unique_ptr<expression>> offset_,
                     std::vector<std::unique_ptr<expression>> shape_,
                     std::vector<std::unique_ptr<expression>> strides_);

    virtual ~array_slice_expr() = default;

    const access_expr& array() const;

    auto offset() const
    {
        return offset_ | boost::adaptors::indirected;
    }

    auto shape() const
    {
        return shape_ | boost::adaptors::indirected;
    }

    auto strides() const
    {
        return strides_ | boost::adaptors::indirected;
    }

    const access_expr& qualified_access() const override final;

    array_slice_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& array_;
        ar& offset_;
        ar& shape_;
        ar& strides_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(array_slice_expr);

private:
    std::unique_ptr<access_expr> array_;
    std::vector<std::unique_ptr<expression>> offset_;
    std::vector<std::unique_ptr<expression>> shape_;
    std::vector<std::unique_ptr<expression>> strides_;
};

bool operator==(const array_slice_expr& lhs, const array_slice_expr& rhs);
bool operator!=(const array_slice_expr& lhs, const array_slice_expr& rhs);

std::unique_ptr<array_slice_expr> slice(std::unique_ptr<access_expr> array,
                                        std::vector<std::unique_ptr<expression>> offset,
                                        std::vector<std::unique_ptr<expression>> shape,
                                        std::vector<std::unique_ptr<expression>> strides);

std::unique_ptr<array_slice_expr> slice(std::unique_ptr<access_expr> array,
                                        std::vector<std::unique_ptr<expression>> offset,
                                        std::vector<std::unique_ptr<expression>> shape);
}

#endif

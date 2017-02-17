#include <qubus/IR/array_slice_expr.hpp>

#include <qubus/IR/literal_expr.hpp>

#include <utility>

namespace qubus
{

array_slice_expr::array_slice_expr(std::unique_ptr<access_expr> array_,
                                   std::vector<std::unique_ptr<expression>> offset_,
                                   std::vector<std::unique_ptr<expression>> shape_,
                                   std::vector<std::unique_ptr<expression>> strides_)
: array_(std::move(array_)),
  offset_(std::move(offset_)),
  shape_(std::move(shape_)),
  strides_(std::move(strides_))
{
}

const access_expr& array_slice_expr::array() const
{
    return *array_;
}

const access_expr& array_slice_expr::qualified_access() const
{
    return array();
}

array_slice_expr* array_slice_expr::clone() const
{
    return new array_slice_expr(qubus::clone(*array_), qubus::clone(offset_),
                                qubus::clone(shape_), qubus::clone(strides_));
}

const expression& array_slice_expr::child(std::size_t index) const
{
    auto rank = shape_.size();

    if (index == 0)
    {
        return *array_;
    }
    else if (index - 1 < rank)
    {
        return *offset_[index - 1];
    }
    else if (index - rank - 1 < rank)
    {
        return *shape_[index - rank - 1];
    }
    else if (index - 2 * rank - 1 < rank)
    {
        return *strides_[index - 2 * rank - 1];
    }
    else
    {
        throw 0;
    }
}

std::size_t array_slice_expr::arity() const
{
    return offset_.size() + shape_.size() + strides_.size() + 1;
}

std::unique_ptr<expression> array_slice_expr::substitute_subexpressions(
    std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() < 1)
        throw 0;

    std::unique_ptr<access_expr> array(dynamic_cast<access_expr*>(new_children[0].release()));

    if (!array)
        throw 0;

    auto rank = shape_.size();

    std::vector<std::unique_ptr<expression>> offset;
    offset.reserve(rank);

    std::vector<std::unique_ptr<expression>> shape;
    shape.reserve(rank);

    std::vector<std::unique_ptr<expression>> strides;
    strides.reserve(rank);

    for (std::size_t i = 1; i < rank + 1; ++i)
    {
        offset.push_back(std::move(new_children[i]));
    }

    for (std::size_t i = rank + 1; i < 2 * rank + 1; ++i)
    {
        shape.push_back(std::move(new_children[i]));
    }

    for (std::size_t i = 2 * rank + 1; i < 3 * rank + 1; ++i)
    {
        strides.push_back(std::move(new_children[i]));
    }

    return std::make_unique<array_slice_expr>(std::move(array), std::move(offset), std::move(shape),
                                              std::move(strides));
}

bool operator==(const array_slice_expr& lhs, const array_slice_expr& rhs)
{
    return lhs.array() == rhs.array() && lhs.offset() == rhs.offset() &&
           lhs.shape() == rhs.shape() && lhs.strides() == rhs.strides();
}

bool operator!=(const array_slice_expr& lhs, const array_slice_expr& rhs)
{
    return !(lhs == rhs);
}

std::unique_ptr<array_slice_expr> slice(std::unique_ptr<access_expr> array,
                                        std::vector<std::unique_ptr<expression>> offset,
                                        std::vector<std::unique_ptr<expression>> shape,
                                        std::vector<std::unique_ptr<expression>> strides)
{
    return std::make_unique<array_slice_expr>(std::move(array), std::move(offset), std::move(shape),
                                              std::move(strides));
}

std::unique_ptr<array_slice_expr> slice(std::unique_ptr<access_expr> array,
                                        std::vector<std::unique_ptr<expression>> offset,
                                        std::vector<std::unique_ptr<expression>> shape)
{
    const auto rank = shape.size();

    std::vector<std::unique_ptr<expression>> strides;
    strides.reserve(rank);

    for (std::size_t i = 0; i < rank; ++i)
    {
        strides.push_back(integer_literal(1));
    }

    return std::make_unique<array_slice_expr>(std::move(array), std::move(offset), std::move(shape),
                                              std::move(strides));
}
}

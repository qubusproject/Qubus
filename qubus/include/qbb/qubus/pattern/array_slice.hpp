#ifndef QUBUS_PATTERN_ARRAY_SLICE_HPP
#define QUBUS_PATTERN_ARRAY_SLICE_HPP

#include <qbb/qubus/IR/array_slice_expr.hpp>

#include <qbb/qubus/pattern/sequence.hpp>
#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/all_of.hpp>
#include <qbb/qubus/pattern/value.hpp>

#include <functional>
#include <utility>

inline namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename Array, typename Offset, typename Shape, typename Strides>
class array_slice_pattern
{
public:
    array_slice_pattern(Array array_, Offset offset_, Shape shape_, Strides strides_)
    : array_(std::move(array_)),
      offset_(std::move(offset_)),
      shape_(std::move(shape_)),
      strides_(std::move(strides_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const array_slice_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<array_slice_expr>())
        {
            if (array_.match(concret_value->array()))
            {
                if (offset_.match(concret_value->offset()))
                {
                    if (shape_.match(concret_value->shape()))
                    {
                        if (strides_.match(concret_value->strides()))
                        {
                            if (var)
                            {
                                var->set(*concret_value);
                            }

                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    void reset() const
    {
        array_.reset();
        offset_.reset();
        shape_.reset();
        strides_.reset();
    }

private:
    Array array_;
    Offset offset_;
    Shape shape_;
    Strides strides_;
};

template <typename Array, typename Offset, typename Shape, typename Strides>
auto array_slice(Array array, Offset offset, Shape shape, Strides strides)
{
    return array_slice_pattern<Array, Offset, Shape, Strides>(
        std::move(array), std::move(offset), std::move(shape), std::move(strides));
}

template <typename Array, typename Offset, typename Shape, typename Strides>
auto array_slice(Array array, Offset offset, Shape shape)
{
    return array_slice(std::move(array), std::move(offset), std::move(shape), all_of(value(1)));
}
}
}
}

#endif

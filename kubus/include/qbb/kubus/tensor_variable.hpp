#ifndef KUBUS_TENSOR_VAR_HPP
#define KUBUS_TENSOR_VAR_HPP

#include <qbb/util/handle.hpp>
#include <qbb/kubus/IR/type.hpp>
#include <qbb/util/integers.hpp>

#include <qbb/kubus/indexed_tensor_expr_context.hpp>

#include <vector>
#include <utility>

namespace qbb
{
namespace kubus
{

class tensor_var
{
public:
    tensor_var(qbb::util::handle handle_, type value_type_, std::vector<qbb::util::index_t> shape_)
    : data_{std::move(handle_)}, value_type_{std::move(value_type_)}, shape_(std::move(shape_))
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    const std::vector<qbb::util::index_t>& shape() const
    {
        return shape_;
    }

    const qbb::util::handle& data_handle() const
    {
        return data_;
    }

private:
    qbb::util::handle data_;
    type value_type_;

    std::vector<qbb::util::index_t> shape_;
};

class tensor : public tensor_expr<typename boost::proto::terminal<tensor_var>::type>
{
public:
    typedef double value_type;

    template <typename... SizeTypes>
    explicit tensor(qbb::util::handle h, SizeTypes... sizes_)
    : tensor::proto_derived_expr{
          tensor::proto_base_expr::make(tensor_var(h, types::double_{}, {sizes_...}))}
    {
    }
};

inline std::ostream& operator<<(std::ostream& os, const tensor_var&)
{
    return os << "tensor ( Rank = " << 2 << " )";
}
}
}

#endif
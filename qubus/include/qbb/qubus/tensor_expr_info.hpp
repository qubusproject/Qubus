#ifndef QBB_QUBUS_TENSOR_EXPR_INFO_HPP
#define QBB_QUBUS_TENSOR_EXPR_INFO_HPP

#include <qbb/qubus/object.hpp>
#include <qbb/qubus/computelet.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/qubus/tensor_expr_closure.hpp>

#include <boost/optional.hpp>

#include <tuple>
#include <vector>

namespace qbb
{
namespace qubus
{

class tensor_expr_info
{
public:
    explicit tensor_expr_info(type result_type_,
                              std::tuple<tensor_expr_closure, std::vector<object>> ir_info);

    computelet stored_computelet() const;

    const std::vector<object>& args() const;

private:
    type result_type_;
    tensor_expr_closure closure_;
    std::vector<object> args_;

    computelet stored_computelet_;
};
}
}

#endif
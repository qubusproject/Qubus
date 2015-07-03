#ifndef QBB_QUBUS_TENSOR_EXPR_INFO_HPP
#define QBB_QUBUS_TENSOR_EXPR_INFO_HPP

#include <qbb/qubus/object.hpp>
#include <qbb/qubus/plan.hpp>

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
                              std::tuple<tensor_expr_closure, std::vector<std::shared_ptr<object>>> ir_info);

    const plan& compiled_plan() const;

    const std::vector<std::shared_ptr<object>>& args() const;

private:
    type result_type_;
    tensor_expr_closure closure_;
    std::vector<std::shared_ptr<object>> args_;

    mutable boost::optional<plan> plan_;
};
}
}

#endif
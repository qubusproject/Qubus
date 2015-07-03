#ifndef QBB_KUBUS_TENSOR_EXPR_INFO_HPP
#define QBB_KUBUS_TENSOR_EXPR_INFO_HPP

#include <qbb/kubus/object.hpp>
#include <qbb/kubus/plan.hpp>

#include <qbb/kubus/IR/type.hpp>

#include <qbb/kubus/tensor_expr_closure.hpp>

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
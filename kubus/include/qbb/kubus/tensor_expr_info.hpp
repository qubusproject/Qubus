#ifndef QBB_KUBUS_TENSOR_EXPR_INFO_HPP
#define QBB_KUBUS_TENSOR_EXPR_INFO_HPP

#include <qbb/kubus/runtime.hpp>

#include <qbb/kubus/IR/function_declaration.hpp>
#include <qbb/kubus/object.hpp>

#include <tuple>
#include <utility>
#include <vector>
#include <boost/optional.hpp>

namespace qbb
{
namespace kubus
{

class tensor_expr_info
{
public:
    explicit tensor_expr_info(std::tuple<function_declaration, std::vector<std::shared_ptr<object>>> ir_info)
    : plan_decl_(std::move(std::get<0>(ir_info))), args_(std::move(std::get<1>(ir_info)))
    {
    }
    
    const plan& compiled_plan() const
    {
        if (!plan_)
        {
            plan_ = get_runtime().compile(plan_decl_);
        }
        
        return *plan_;
    }
    
    const std::vector<std::shared_ptr<object>>& args() const
    {
        return args_;
    }
private:
    function_declaration plan_decl_;
    std::vector<std::shared_ptr<object>> args_;
    
    mutable boost::optional<plan> plan_;
};
}
}

#endif
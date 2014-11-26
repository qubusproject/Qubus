#include <qbb/kubus/IR/function_declaration.hpp>

#include <qbb/kubus/IR/type_inference.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

class function_declaration_info
{
public:
    explicit function_declaration_info(std::vector<variable_declaration> params_,
                                       variable_declaration result_, expression body_)
    : params_(std::move(params_)), result_(std::move(result_)), body_(std::move(body_))
    {
    }

    function_declaration_info(const function_declaration_info&) = delete;
    function_declaration_info& operator=(const function_declaration_info&) = delete;

    const std::vector<variable_declaration>& params() const
    {
        return params_;
    }
    
    const variable_declaration& result() const
    {
        return result_;
    }
    
    const expression& body() const
    {
        return body_;
    }

    annotation_map& annotations() const
    {
        return annotations_;
    }

    annotation_map& annotations()
    {
        return annotations_;
    }

private:
    std::vector<variable_declaration> params_;
    variable_declaration result_;
    expression body_;

    mutable annotation_map annotations_;
};

function_declaration::function_declaration(std::vector<variable_declaration> params_,
                                           variable_declaration result_, expression body_)
: info_(std::make_shared<function_declaration_info>(std::move(params_), std::move(result_),
                                                    std::move(body_)))
{
}

const std::vector<variable_declaration>& function_declaration::params() const
{
    return info_->params();
}

const variable_declaration& function_declaration::result() const
{
    return info_->result();
}

const expression& function_declaration::body() const
{
    return info_->body();
}

util::handle function_declaration::id() const
{
    return util::handle_from_ptr(info_.get());
}

annotation_map& function_declaration::annotations() const
{
    return info_->annotations();
}

annotation_map& function_declaration::annotations()
{
    return info_->annotations();
}

bool operator==(const function_declaration& lhs, const function_declaration& rhs)
{
    return lhs.id() == rhs.id();
}

bool operator!=(const function_declaration& lhs, const function_declaration& rhs)
{
    return !(lhs == rhs);
}
}
}
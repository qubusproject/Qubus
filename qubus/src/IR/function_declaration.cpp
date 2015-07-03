#include <qbb/qubus/IR/function_declaration.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

class function_declaration_info
{
public:
    explicit function_declaration_info(std::string name_, std::vector<variable_declaration> params_,
                                       variable_declaration result_, expression body_)
    : name_(std::move(name_)), params_(std::move(params_)), result_(std::move(result_)),
      body_(std::move(body_))
    {
    }

    function_declaration_info(const function_declaration_info&) = delete;
    function_declaration_info& operator=(const function_declaration_info&) = delete;

    const std::string& name() const
    {
        return name_;
    }

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
    
    void substitute_body(expression body)
    {
        body_ = std::move(body);
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
    std::string name_;
    std::vector<variable_declaration> params_;
    variable_declaration result_;
    expression body_;

    mutable annotation_map annotations_;
};

function_declaration::function_declaration(std::string name_,
                                           std::vector<variable_declaration> params_,
                                           variable_declaration result_, expression body_)
: info_(std::make_shared<function_declaration_info>(std::move(name_), std::move(params_),
                                                    std::move(result_), std::move(body_)))
{
}

const std::string& function_declaration::name() const
{
    return info_->name();
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

void function_declaration::substitute_body(expression body)
{
    info_->substitute_body(std::move(body));
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
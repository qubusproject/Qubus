#ifndef QBB_QUBUS_FUNCTION_DECLARATION_HPP
#define QBB_QUBUS_FUNCTION_DECLARATION_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/annotations.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/include/serialization.hpp>

#include <boost/optional.hpp>

#include <memory>
#include <vector>

inline namespace qbb
{
namespace qubus
{

class function_declaration_info
{
public:
    function_declaration_info() = default;
    explicit function_declaration_info(std::string name_, std::vector<variable_declaration> params_,
                                       variable_declaration result_, std::unique_ptr<expression> body_);

    function_declaration_info(const function_declaration_info&) = delete;
    function_declaration_info& operator=(const function_declaration_info&) = delete;

    const std::string& name() const;

    const std::vector<variable_declaration>& params() const;

    const variable_declaration& result() const;

    std::size_t arity() const;

    const expression& body() const;

    void substitute_body(std::unique_ptr<expression> body);

    annotation_map& annotations() const;

    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & name_;
        ar & params_;
        ar & result_;
        ar & body_;
    }
private:
    std::string name_;
    std::vector<variable_declaration> params_;
    variable_declaration result_;
    std::unique_ptr<expression> body_;

    mutable annotation_map annotations_;
};

// TODO: add may_alias attribute
class function_declaration
{
public:
    function_declaration() = default;
    function_declaration(std::string name_, std::vector<variable_declaration> params_,
                         variable_declaration result_, std::unique_ptr<expression> body_);

    const std::string& name() const;
    const std::vector<variable_declaration>& params() const;
    const variable_declaration& result() const;

    std::size_t arity() const;
    
    void substitute_body(std::unique_ptr<expression> body);

    const expression& body() const;

    util::handle id() const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & info_;
    }
private:
    std::shared_ptr<function_declaration_info> info_;
};

bool operator==(const function_declaration& lhs, const function_declaration& rhs);
bool operator!=(const function_declaration& lhs, const function_declaration& rhs);
}
}

#endif
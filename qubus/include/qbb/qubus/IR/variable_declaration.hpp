#ifndef QBB_QUBUS_VARIABLE_DECLARATION_HPP
#define QBB_QUBUS_VARIABLE_DECLARATION_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/annotations.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/unused.hpp>
#include <hpx/include/serialization.hpp>

#include <memory>
#include <functional>

namespace qbb
{
namespace qubus
{

enum class variable_intent
{
    generic,
    in,
    out
};

class variable_declaration_info
{
public:
    variable_declaration_info();
    explicit variable_declaration_info(type var_type_, variable_intent intent_);

    variable_declaration_info(const variable_declaration_info&) = delete;
    variable_declaration_info& operator=(const variable_declaration_info&) = delete;

    const type& var_type() const;

    variable_intent intent() const;

    annotation_map& annotations() const;

    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & var_type_;
        ar & intent_;
    }
private:
    type var_type_;
    variable_intent intent_;

    mutable annotation_map annotations_;
};

class variable_declaration
{
public:
    variable_declaration() = default;
    explicit variable_declaration(type var_type_, variable_intent intent_ = variable_intent::generic);
    
    const type& var_type() const;
    variable_intent intent() const;
    util::handle id() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & info_;
    }
private:
    std::shared_ptr<variable_declaration_info> info_;
};

bool operator==(const variable_declaration& lhs, const variable_declaration& rhs);
bool operator!=(const variable_declaration& lhs, const variable_declaration& rhs);

}
}

namespace std
{

template<>
struct less<qbb::qubus::variable_declaration>
{
    using result_type = bool;
    using first_argument_type = qbb::qubus::variable_declaration;
    using second_argument_type = first_argument_type;

    bool operator()(const qbb::qubus::variable_declaration& lhs, const qbb::qubus::variable_declaration& rhs) const
    {
        return lhs.id() < rhs.id();
    }
};

}

#endif
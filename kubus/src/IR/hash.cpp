#include <qbb/kubus/IR/hash.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/hash.hpp>

#include <map>
#include <typeindex>

namespace qbb
{
namespace kubus
{

namespace
{

class hash_context
{
public:
    util::handle get_unified_id(const util::handle& orig_id)
    {
        auto iter = variable_table_.find(orig_id);

        if (iter != variable_table_.end())
        {
            return iter->second;
        }
        else
        {
            util::handle unified_id(variable_table_.size());

            variable_table_.emplace(orig_id, unified_id);

            return unified_id;
        }
    }

private:
    std::map<util::handle, util::handle> variable_table_;
};

std::size_t hash(const expression& expr, hash_context& ctx)
{
    using pattern::_;

    pattern::variable<expression> a, b, c;
    pattern::variable<variable_declaration> decl;
    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<type> type;
    pattern::variable<std::string> name;
    pattern::variable<double> dval;
    pattern::variable<float> fval;
    pattern::variable<util::index_t> ival;

    std::size_t seed = 0;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(variable_ref(decl),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(variable_ref_expr)));
                       util::hash_combine(seed, ctx.get_unified_id(decl.get().id()));
                   })
            .case_(for_(decl, _, _, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(for_expr)));
                       util::hash_combine(seed, ctx.get_unified_id(decl.get().id()));
                   })
            .case_(for_all(decl, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(for_all_expr)));
                       util::hash_combine(seed, ctx.get_unified_id(decl.get().id()));
                   })
            .case_(binary_operator(btag, _, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(binary_operator_expr)));
                       util::hash_combine(seed, static_cast<std::size_t>(btag.get()));
                   })
            .case_(unary_operator(utag, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(unary_operator_expr)));
                       util::hash_combine(seed, static_cast<std::size_t>(utag.get()));
                   })
            .case_(type_conversion(type, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(type_conversion_expr)));
                       util::hash_combine(seed, type.get());
                   })
            .case_(subscription(_, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(subscription_expr)));
                   })
            .case_(compound(_),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(compound_expr)));
                   })
            .case_(intrinsic_function(name, _),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(intrinsic_function_expr)));
                       util::hash_combine(seed, name.get());
                   })
            .case_(double_literal(dval),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(double_literal_expr)));
                       util::hash_combine(seed, dval.get());
                   })
            .case_(float_literal(fval),
                   [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(float_literal_expr)));
                       util::hash_combine(seed, fval.get());
                   })
            .case_(integer_literal(ival), [&]
                   {
                       util::hash_combine(seed, std::type_index(typeid(integer_literal_expr)));
                       util::hash_combine(seed, ival.get());
                   });

    pattern::for_each(expr, m);

    return seed;
}
}

std::size_t hash(const function_declaration& decl)
{
    if (const auto& hash_value = decl.annotations().lookup("kubus.hash"))
    {
        return hash_value.as<std::size_t>();
    }
    else
    {
        hash_context ctx;
        
        std::size_t seed = 0;
        
        for (const auto& param : decl.params())
        {
            util::hash_combine(seed, ctx.get_unified_id(param.id()));
            util::hash_combine(seed, param.var_type());
            util::hash_combine(seed, static_cast<std::size_t>(param.intent()));
        }

        std::size_t computed_hash_value = hash(decl.body(), ctx);
        
        util::hash_combine(seed, computed_hash_value);

        decl.annotations().add("kubus.hash", annotation(seed));

        return seed;
    }
}
}
}
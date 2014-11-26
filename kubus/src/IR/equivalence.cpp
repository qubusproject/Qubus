#include <qbb/kubus/IR/equivalence.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/assert.hpp>

#include <map>

namespace qbb
{
namespace kubus
{

namespace
{
class equivalence_test_context
{
public:
    bool test_equivalence(const util::handle& lhs_id, const util::handle& rhs_id)
    {
        auto result = lhs_to_rhs_id_map_.emplace(lhs_id, rhs_id);
        
        if (!result.second)
        {
            return result.first->second == rhs_id;
        }
        else
        {
            return true;
        }
    }
private:
    std::map<util::handle, util::handle> lhs_to_rhs_id_map_;
};

bool test_equivalence(const expression& lhs, const expression& rhs, equivalence_test_context& ctx)
{
    // TODO: implement this
    return false;
}

}

bool test_equivalence(const function_declaration& lhs, const function_declaration& rhs)
{
    equivalence_test_context ctx;

    if (lhs.params().size() != rhs.params().size())
        return false;

    auto num_params = lhs.params().size();

    const auto& lhs_params = lhs.params();
    const auto& rhs_params = rhs.params();

    for (std::size_t i = 0; i < num_params; ++i)
    {
        if (lhs_params[i].var_type() != rhs_params[i].var_type())
            return false;
        
        if (lhs_params[i].intent() != rhs_params[i].intent())
            return false;
        
        QBB_VERIFY(ctx.test_equivalence(lhs_params[i].id(), rhs_params[i].id()),
                   "Function parameters are always equivalent.");
    }
    
    return test_equivalence(lhs.body(), rhs.body(), ctx);
}
}
}

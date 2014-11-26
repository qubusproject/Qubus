#ifndef QBB_KUBUS_PLAN_HPP
#define QBB_KUBUS_PLAN_HPP

#include <qbb/util/handle.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{

enum class intent
{
in,
inout
};
    
class plan
{
public:
    explicit plan(const util::handle& id_, std::vector<intent> intents_);
    
    const util::handle& id() const;
    
    const std::vector<intent>& intents() const;
private:
    util::handle id_;
    std::vector<intent> intents_;
};
    
}
}

#endif
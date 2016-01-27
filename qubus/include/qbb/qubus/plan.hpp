#ifndef QBB_QUBUS_PLAN_HPP
#define QBB_QUBUS_PLAN_HPP

#include <qbb/util/handle.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
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

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & id_;
        ar & intents_;
    }
private:
    util::handle id_;
    std::vector<intent> intents_;
};
    
}
}

#endif
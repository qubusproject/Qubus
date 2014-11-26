#include <qbb/kubus/plan.hpp>

namespace qbb
{
namespace kubus
{

plan::plan(const util::handle& id_, std::vector<intent> intents_)
: id_(id_), intents_(intents_)
{
}

const util::handle& plan::id() const
{
    return id_;
}

const std::vector<intent>& plan::intents() const
{
    return intents_;
}

}
}
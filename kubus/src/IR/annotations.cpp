#include <qbb/kubus/IR/annotations.hpp>

namespace qbb
{
namespace kubus
{
bool annotation::has_value() const
{
    return !!value_;
}
    
annotation::operator bool() const
{
    return has_value();
}

void annotation_map::add(std::string key, annotation value)
{
    annotations_.emplace(std::move(key), std::move(value));
}
    
annotation annotation_map::lookup(const std::string& key) const
{
    auto iter = annotations_.find(key);
    
    if(iter != annotations_.end())
    {
        return iter->second;
    }
    else
    {
        return annotation{};
    }
}
    
}    
}
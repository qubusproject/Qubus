#ifndef QBB_KUBUS_ANNOTATIONS_HPP
#define QBB_KUBUS_ANNOTATIONS_HPP

#include <boost/any.hpp>

#include <map>
#include <string>
#include <memory>
#include <utility>

namespace qbb
{
namespace kubus
{
 
class annotation
{
public:
    annotation() = default;
    
    template<typename T>
    explicit annotation(T value_)
    : value_{std::make_shared<boost::any>(std::move(value_))}
    {
    }
    
    template<typename T>
    const T as() const
    {
        return boost::any_cast<T>(*value_);
    }
    
    template<typename T>
    T as()
    {
        return boost::any_cast<T>(*value_);
    }
    
    bool has_value() const;
    
    explicit operator bool() const;
private:
    std::shared_ptr<boost::any> value_;
};

class annotation_map
{
public:
    void add(std::string key, annotation value);
    
    annotation lookup(const std::string& key) const;
private:
    std::map<std::string, annotation> annotations_;
};
    
}   
}


#endif
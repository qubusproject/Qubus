#ifndef QUBUS_ANNOTATIONS_HPP
#define QUBUS_ANNOTATIONS_HPP

#include <qbb/util/assert.hpp>

#include <boost/any.hpp>

#include <map>
#include <string>
#include <memory>
#include <utility>

namespace qubus
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
        QBB_ASSERT(has_value(), "Annotation has no value.");

        return boost::any_cast<T>(*value_);
    }
    
    template<typename T>
    T as()
    {
        QBB_ASSERT(has_value(), "Annotation has no value.");

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


#endif
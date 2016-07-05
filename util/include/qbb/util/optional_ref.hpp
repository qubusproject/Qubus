#ifndef QBB_UTIL_OPTIONAL_REF_HPP
#define QBB_UTIL_OPTIONAL_REF_HPP

#include <boost/optional.hpp>
#include <type_traits>

namespace qbb
{
namespace util
{
    
template<typename T>
class optional_ref
{
public:
    using value_type = typename std::remove_cv<T>::type;
    
    optional_ref()
    : ref_(nullptr)
    {
    }
    
    optional_ref(T& ref_)
    : ref_(&ref_)
    {
    }

    optional_ref(T* ref_)
    : ref_(ref_)
    {
    }

    optional_ref(boost::optional<value_type>& ref_)
    : ref_(ref_ ? &ref_.get() : nullptr)
    {
    }
    
    /*optional_ref(typename std::enable_if<std::is_const<T>::value ,const boost::optional<value_type>&>::type ref_)
    : ref_(ref_ ? &ref_.get() : nullptr)
    {
    }*/
    
    optional_ref<T>& operator=(T& ref_)
    {
        this->ref_ = ref_;
        
        return *this;
    }
    
    T& get() const
    {
        return *ref_;
    }
    
    T& operator*() const
    {
        return get();
    }

    T* operator->() const
    {
        return ref_;
    }
    
    explicit operator bool() const
    {
        return ref_;
    }
    
    operator boost::optional<value_type>() const
    {
        if(ref_)
        {
            return *ref_;
        }
        else
        {
            return {};
        }
    }
private:
    T* ref_;
};

}
}

#endif
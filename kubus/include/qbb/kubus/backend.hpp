#ifndef QBB_QUBUS_BACKEND_HPP
#define QBB_QUBUS_BACKEND_HPP

#include <hpx/config.hpp>

#include <qbb/kubus/local_address_space.hpp>
#include <qbb/kubus/local_object_factory.hpp>

#include <qbb/kubus/executor.hpp>
#include <qbb/kubus/compiler.hpp>

#include <vector>
#include <string>
#include <memory>

namespace qbb
{
namespace qubus
{

class backend
{
public:
    backend() = default;
    backend(const backend&) = delete;
    
    virtual ~backend() = default;
    
    backend& operator=(const backend&) = delete;
    
    //TODO: Substitute string with string_view
    virtual std::string id() const = 0;
    
    virtual std::vector<executor*> executors() const = 0;    
    virtual compiler& get_compiler() const = 0;
    
    virtual local_object_factory& local_factory() const = 0;
    virtual local_address_space& address_space() const = 0;
    
    //virtual void connect(const backend& other) = 0;
};

}   
}

#endif
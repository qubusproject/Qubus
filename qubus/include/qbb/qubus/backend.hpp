#ifndef QBB_QUBUS_BACKEND_HPP
#define QBB_QUBUS_BACKEND_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/local_address_space.hpp>
#include <qbb/qubus/local_object_factory.hpp>

#include <qbb/qubus/executor.hpp>
#include <qbb/qubus/compiler.hpp>

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
};

}   
}

#endif
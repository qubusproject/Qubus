#ifndef QUBUS_BACKEND_HPP
#define QUBUS_BACKEND_HPP

#include <hpx/config.hpp>

#include <qubus/vpu.hpp>

#include <vector>
#include <string>
#include <memory>

namespace qubus
{

enum class backend_type : unsigned int
{
    vpu = 0,
    host = 1
};

class backend
{
public:
    backend() = default;
    backend(const backend&) = delete;
    
    virtual ~backend() = default;
    
    backend& operator=(const backend&) = delete;
    
    //TODO: Substitute string with string_view
    virtual std::string id() const = 0;

    virtual std::vector<std::unique_ptr<vpu>> create_vpus() const = 0;
};

}

#endif
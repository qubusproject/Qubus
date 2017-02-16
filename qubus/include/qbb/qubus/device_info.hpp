#ifndef QBB_QUBUS_DEVICE_INFO_HPP
#define QBB_QUBUS_DEVICE_INFO_HPP

#include <cstddef>
#include <vector>
#include <string>

namespace qubus
{
 
class device_info
{
public:
    virtual ~device_info() = default;
    
    device_info(const device_info&) = delete;
    device_info& operator=(const device_info&) = delete;
    
    virtual std::vector<std::string> get_features() = 0;
};

}

#endif
#ifndef QBB_KUBUS_CPU_BACKEND_HPP
#define QBB_KUBUS_CPU_BACKEND_HPP

#include <qbb/kubus/backend.hpp>

namespace qbb
{
namespace kubus
{

class abi_info;
    
extern "C" backend* init_cpu_backend(const abi_info* abi);
    
}   
}

#endif
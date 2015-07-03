#ifndef QBB_QUBUS_CPU_BACKEND_HPP
#define QBB_QUBUS_CPU_BACKEND_HPP

#include <qbb/qubus/backend.hpp>

namespace qbb
{
namespace qubus
{

class abi_info;
    
extern "C" backend* init_cpu_backend(const abi_info* abi);
    
}   
}

#endif
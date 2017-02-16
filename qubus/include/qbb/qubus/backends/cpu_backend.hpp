#ifndef QUBUS_CPU_BACKEND_HPP
#define QUBUS_CPU_BACKEND_HPP

#include <qbb/qubus/backend.hpp>

namespace qubus
{

class abi_info;

extern "C" unsigned long int cpu_backend_get_api_version();
extern "C" backend* init_cpu_backend(const abi_info* abi);
    
}

#endif
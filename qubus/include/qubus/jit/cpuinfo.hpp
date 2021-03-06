#ifndef QUBUS_JIT_CPUINFO_HPP
#define QUBUS_JIT_CPUINFO_HPP

#include <vector>
#include <string>
#include <qubus/util/integers.hpp>

namespace qubus
{

std::vector<std::string> deduce_host_cpu_features();
std::vector<std::string> get_host_cpu_features();
util::index_t get_prefered_alignment();

}

#endif
#ifndef QBB_KUBUS_CPUINFO_HPP
#define QBB_KUBUS_CPUINFO_HPP

#include <vector>
#include <string>
#include <qbb/util/integers.hpp>

namespace qbb
{
namespace kubus
{

std::vector<std::string> deduce_host_cpu_features();
util::index_t get_prefered_alignment();

}   
}

#endif
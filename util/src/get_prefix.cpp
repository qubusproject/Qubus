#include <qbb/util/get_prefix.hpp>

#include <qbb/util/dll.hpp>

namespace qbb
{
namespace util
{
 
boost::filesystem::path get_prefix(const std::string& name)
{
    dll this_module("lib" + name + ".so");

    return this_module.get_directory();
}
    
}   
}
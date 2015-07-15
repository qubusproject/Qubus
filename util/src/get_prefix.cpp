#include <qbb/util/get_prefix.hpp>

#include <boost/dll.hpp>

namespace qbb
{
namespace util
{
 
boost::filesystem::path get_prefix(const std::string& name)
{
    boost::dll::shared_library this_module(name, boost::dll::load_mode::append_decorations);

    return this_module.location().parent_path();
}
    
}   
}
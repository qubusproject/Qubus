#include <qubus/util/get_prefix.hpp>

#include <boost/dll.hpp>

namespace qubus
{
namespace util
{

boost::filesystem::path get_prefix(const std::string& name)
{
    using namespace boost::dll;

    shared_library this_module(name,
                               load_mode::append_decorations | load_mode::search_system_folders);

    return this_module.location().parent_path();
}
}
}
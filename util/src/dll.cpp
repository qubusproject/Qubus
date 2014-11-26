#include <qbb/util/dll.hpp>

#include <array>

#include <link.h>
#include <dlfcn.h>
#include <limits.h>

namespace qbb
{
namespace util
{

dll::dll(const std::string& filename)
{
    //flush error state
    dlerror();
    
    handle_ = dlopen(filename.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    
    if(!handle_)
    {
        throw exception(dlerror());
    }
}

dll::~dll()
{
    //TODO: Error handling ?
    dlclose(handle_);
}

void* dll::lookup_symbol_raw(const std::string& symbol) const
{
    //flush error state
    dlerror();
    
    void* result = dlsym(handle_, symbol.c_str());
    
    if(auto err_msg = dlerror())
    {
        throw exception(err_msg);
    }
    
    return result;
}

boost::filesystem::path dll::get_directory() const
{
    char directory[PATH_MAX] = {'\0'};
   
    dlerror();
    
    if (dlinfo(handle_, RTLD_DI_ORIGIN, directory) < 0)
    {
        throw exception(dlerror());
    }
    
    return boost::filesystem::path(directory);
}

}
}
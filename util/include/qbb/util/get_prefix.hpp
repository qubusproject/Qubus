#ifndef QBB_UTIL_GET_PREFIX_HPP
#define QBB_UTIL_GET_PREFIX_HPP

#include <boost/filesystem/path.hpp>

#include <string>

namespace qubus
{
namespace util
{

boost::filesystem::path get_prefix(const std::string& name);
    
}   
}

#endif
#include <boost/preprocessor/stringize.hpp>

namespace qubus
{
const char* get_llvm_version_string()
{
    return BOOST_PP_STRINGIZE(LLVM_VERSION_MAJOR) "." BOOST_PP_STRINGIZE(LLVM_VERSION_MINOR);
}
}
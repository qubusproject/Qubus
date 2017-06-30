#include <qubus/prefix.hpp>

#include <boost/preprocessor/stringize.hpp>

boost::filesystem::path get_prefix()
{
    const char* prefix = BOOST_PP_STRINGIZE(QUBUS_PREFIX);

    return prefix;
}
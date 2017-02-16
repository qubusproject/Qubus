#include <qbb/util/unique_name_generator.hpp>

inline namespace qbb
{
namespace util
{

unique_name_generator::unique_name_generator() : next_name_("A")
{
}

std::string unique_name_generator::get() const
{
    std::string result = next_name_;

    ++next_name_.back();

    return result;
}
}
}
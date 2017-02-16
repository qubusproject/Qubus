#include <qubus/util/unique_name_generator.hpp>

namespace qubus
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
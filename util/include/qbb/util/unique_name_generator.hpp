#ifndef QBB_UTIL_UNIQUE_NAME_GENERATOR_HPP
#define QBB_UTIL_UNIQUE_NAME_GENERATOR_HPP

#include <string>

inline namespace qbb
{
namespace util
{
class unique_name_generator
{
public:
    unique_name_generator();

    std::string get() const;

private:
    mutable std::string next_name_;
};
}
}

#endif
#ifndef QUBUS_PERFORMANCE_ESTIMATE_HPP
#define QUBUS_PERFORMANCE_ESTIMATE_HPP

#include <qubus/util/hpx/serialization/chrono.hpp>

#include <chrono>

namespace qubus
{

struct performance_estimate
{
    std::chrono::microseconds runtime;
    std::chrono::microseconds accuracy;

    template<typename Archive>
    void serialize(Archive &ar, unsigned QUBUS_UNUSED(version))
    {
        ar & runtime;
        ar & accuracy;
    }
};

}

#endif

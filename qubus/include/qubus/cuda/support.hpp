#ifndef QUBUS_CUDA_SUPPORT_HPP
#define QUBUS_CUDA_SUPPORT_HPP

#include <qubus/cuda/core.hpp>

#include <hpx/include/lcos.hpp>

namespace qubus
{
namespace cuda
{

hpx::future<void> when_finished(stream& s);

}
}

#endif

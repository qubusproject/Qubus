#ifndef QUBUS_CUDA_ARCH_HPP
#define QUBUS_CUDA_ARCH_HPP

#include <qubus/architecture_identifier.hpp>

namespace qubus
{

class cuda_architecture_identifier : public architecture_identifier_base<cuda_architecture_identifier>
{
public:
    constexpr explicit cuda_architecture_identifier(int device_id_)
    : device_id_(device_id_)
    {
    }

    constexpr util::span<const char> payload() const
    {
        return util::span<const char>(reinterpret_cast<const char*>(&device_id_), sizeof(device_id_));
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar & device_id_;
    }

private:
    int device_id_;
};

namespace arch
{
using cuda_type = cuda_architecture_identifier;

constexpr auto cuda(int device_id)
{
    return cuda_architecture_identifier(device_id);
}
}

}

#endif

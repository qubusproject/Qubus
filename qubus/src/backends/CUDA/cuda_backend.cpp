#include <qubus/backend.hpp>

#include <qubus/abi_info.hpp>

#include <qubus/qubus_export.h>

#include <memory>
#include <mutex>

namespace qubus
{

class cuda_backend final : public backend
{
public:
    cuda_backend(const abi_info& abi_) : abi_(&abi_)
    {
    }

    ~cuda_backend() override = default;

    std::string id() const override
    {
        return "qubus.cuda";
    }

    std::vector<std::unique_ptr<vpu>> create_vpus() const override
    {
        return {};
    }

private:
    const abi_info* abi_;
};

extern "C" QUBUS_EXPORT unsigned int cuda_backend_get_backend_type()
{
    return static_cast<unsigned int>(backend_type::vpu);
}

extern "C" QUBUS_EXPORT unsigned long int cuda_backend_get_api_version()
{
    return 0;
}

namespace
{
std::unique_ptr<cuda_backend> the_cuda_backend;
std::once_flag cuda_backend_init_flag;
}

extern "C" QUBUS_EXPORT backend* init_cuda_backend(const abi_info* abi)
{
    std::call_once(cuda_backend_init_flag,
                   [&] { the_cuda_backend = std::make_unique<cuda_backend>(*abi); });

    return the_cuda_backend.get();
}
}
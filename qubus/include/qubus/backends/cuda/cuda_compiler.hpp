#ifndef QUBUS_BACKENDS_CUDA_CUDA_COMPILER_HPP
#define QUBUS_BACKENDS_CUDA_CUDA_COMPILER_HPP

#include <hpx/config.hpp>

#include <qubus/cuda/core.hpp>

#include <qubus/IR/module.hpp>
#include <qubus/IR/symbol_id.hpp>

#include <memory>
#include <functional>
#include <vector>

namespace qubus
{

class cuda_runtime;

class cuda_plan
{
public:
    cuda_plan() = default;
    virtual ~cuda_plan() = default;

    cuda_plan(const cuda_plan&) = delete;
    cuda_plan& operator=(const cuda_plan&) = delete;

    virtual void execute(const symbol_id& entry_point, const std::vector<void*>& args, cuda::stream& stream) const = 0;
};

class cuda_compiler
{
public:
    cuda_compiler();
    ~cuda_compiler();

    cuda_compiler(const cuda_compiler&) = delete;
    cuda_compiler(cuda_compiler&&) = delete;

    cuda_compiler& operator=(const cuda_compiler&) = delete;
    cuda_compiler& operator=(cuda_compiler&&) = delete;

    std::unique_ptr<cuda_plan> compile_computelet(std::unique_ptr<module> computelet);
private:
    class impl;

    std::unique_ptr<impl> impl_;
};
}

#endif

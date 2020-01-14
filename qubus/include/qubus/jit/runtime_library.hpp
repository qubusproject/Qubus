#ifndef QUBUS_JIT_RUNTIME_LIBRARY_HPP
#define QUBUS_JIT_RUNTIME_LIBRARY_HPP

#include <qubus/jit/jit_engine.hpp>

#include <qubus/IR/type.hpp>

#include <llvm/IR/Module.h>

#include <qubus/util/integers.hpp>

#include <memory>

namespace qubus::jit
{

class compiler;

struct runtime_function
{
    explicit runtime_function(std::string name, std::vector<type> parameter_types, llvm::Function* implementation)
    : name(std::move(name)), parameter_types(std::move(parameter_types)), implementation(implementation)
    {
    }

    std::string name;
    std::vector<type> parameter_types;
    llvm::Function* implementation;
};

class runtime_library
{
public:
    explicit runtime_library(compiler& compiler_);

    ~runtime_library();

    runtime_library(const runtime_library&) = delete;
    runtime_library(runtime_library&&) = default;

    runtime_library& operator=(const runtime_library&) = delete;
    runtime_library& operator=(runtime_library&&) = default;

    llvm::Type* map_qubus_type(const type& t) const;

    runtime_function get_array_constructor(const type& value_type, util::index_t rank,
                                           llvm::Module& mod);
    runtime_function get_array_sizeof(const type& value_type, util::index_t rank,
                                      llvm::Module& mod);

    class impl;

private:
    std::unique_ptr<impl> impl_;
};

} // namespace qubus::jit

#endif

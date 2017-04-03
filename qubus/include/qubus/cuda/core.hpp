#ifndef QUBUS_CUDA_CORE_HPP
#define QUBUS_CUDA_CORE_HPP

#include <cuda.h>

#include <exception>
#include <functional>
#include <list>
#include <string>
#include <vector>

namespace qubus
{
namespace cuda
{

class exception : public virtual std::exception
{
};

class cuda_error : public virtual exception
{
public:
    explicit cuda_error(CUresult error_code_);

    CUresult error_code() const;

    const char* what() const noexcept override;

private:
    CUresult error_code_;
};

void check_cuda_error(CUresult result);

void init();

struct compute_capability_info
{
    compute_capability_info(int major_revision, int minor_revision)
    : major_revision(major_revision), minor_revision(minor_revision)
    {
    }

    int major_revision;
    int minor_revision;
};

class device
{
public:
    explicit device(int ordinal);

    device(const device&) = delete;
    device& operator=(const device&) = delete;

    device(device&& other);
    device& operator=(device&& other);

    std::string name() const;

    std::size_t total_mem() const;

    compute_capability_info compute_capability() const;

    int get_attribute(CUdevice_attribute attribute) const;

    int max_threads_per_block() const;
    int max_shared_memory_per_block() const;
    int warp_size() const;
    int max_block_dim_x() const;
    int max_grid_dim_x() const;

    CUdevice native_handle() const;

private:
    CUdevice device_;
};

int get_device_count();
std::vector<device> get_devices();

class context
{
public:
    context();

    explicit context(const device& dev);

    context(const context&) = delete;
    context& operator=(const context&) = delete;

    context(context&& other);

    context& operator=(context&& other);

    ~context();

    CUcontext native_handle() const;

private:
    CUcontext context_;
};

namespace this_context
{
void synchronize();
}

class stream
{
public:
    using callback_closure = std::function<void()>;

    stream();

    stream(const stream&) = delete;
    stream& operator=(const stream&) = delete;

    stream(stream&& other);

    stream& operator=(stream&& other);

    ~stream();

    void add_callback(std::function<void()> callback);

    CUstream native_handle() const;

private:
    CUstream stream_handle_;
    std::list<stream::callback_closure> pending_callbacks_;
};

struct architecture_version
{
    architecture_version(int major_version, int minor_version)
    : major_version(major_version), minor_version(minor_version)
    {
    }

    int major_version;
    int minor_version;
};

bool operator==(const architecture_version& lhs, const architecture_version& rhs);
bool operator!=(const architecture_version& lhs, const architecture_version& rhs);

bool operator<(const architecture_version& lhs, const architecture_version& rhs);
bool operator>(const architecture_version& lhs, const architecture_version& rhs);
bool operator<=(const architecture_version& lhs, const architecture_version& rhs);
bool operator>=(const architecture_version& lhs, const architecture_version& rhs);

class function
{
public:
    explicit function(CUfunction function_);

    int get_attribute(CUfunction_attribute attr) const;

    int max_threads_per_block() const;
    int shared_memory_size() const;
    int number_of_register_per_thread() const;
    architecture_version ptx_version() const;
    architecture_version binary_version() const;

    void set_cache_config(CUfunc_cache config);
    void set_shared_memory_config(CUsharedconfig config);

    CUfunction native_handle() const;

private:
    CUfunction function_;
};

template <typename... Parameters>
void launch_kernel(const function& kernel, std::size_t grid_extend, std::size_t block_extend,
                   std::size_t shared_memory_size, Parameters&&... parameters)
{
    void* params[] = {&parameters...};

    cuLaunchKernel(kernel.native_handle(), grid_extend, 1, 1, block_extend, 1, 1,
                   shared_memory_size, 0, params, 0);
}

template <typename... Parameters>
void launch_kernel(const function& kernel, std::size_t grid_extend, std::size_t block_extend,
                   std::size_t shared_memory_size, const stream& used_stream,
                   Parameters&&... parameters)
{
    void* params[] = {&parameters...};

    cuLaunchKernel(kernel.native_handle(), grid_extend, 1, 1, block_extend, 1, 1,
                   shared_memory_size, used_stream.native_handle(), params, 0);
}

int calculate_max_active_blocks_per_multiprocessor(const function& kernel, int block_size,
                                                   std::size_t dynamic_shared_mem_size);

struct suggested_launch_config
{
    suggested_launch_config(int min_grid_size, int block_size)
    : min_grid_size(min_grid_size), block_size(block_size)
    {
    }

    int min_grid_size;
    int block_size;
};

suggested_launch_config
calculate_launch_config_with_max_occupancy(const function& kernel,
                                           std::size_t dynamic_shared_mem_size);

class module
{
public:
    explicit module(const std::string& ptx_code);

    module(const module&) = delete;
    module& operator=(const module&) = delete;

    module(module&& other);
    module& operator=(module&& other);

    ~module();

    const std::string& log_message() const;
    const std::string& error_message() const;

    function get_function(const std::string& name) const;

    CUmodule native_handle() const;

private:
    CUmodule module_;
    std::string log_msg_;
    std::string error_msg_;
};

using device_ptr = CUdeviceptr;

device_ptr device_malloc(std::size_t size);
void device_free(device_ptr ptr);

void memcpy(void* dst, device_ptr src, std::size_t size);
void memcpy(device_ptr dst, void* src, std::size_t size);

class unique_device_ptr
{
public:
    unique_device_ptr(device_ptr ptr_);

    unique_device_ptr(const unique_device_ptr&) = delete;
    unique_device_ptr& operator=(const unique_device_ptr&) = delete;

    unique_device_ptr(unique_device_ptr&& other);

    unique_device_ptr& operator=(unique_device_ptr&& other);

    ~unique_device_ptr();

    device_ptr get();

private:
    device_ptr ptr_;
};
}
}

#endif

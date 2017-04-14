#include <qubus/cuda/core.hpp>

#include <qubus/util/assert.hpp>

#include <cstring>
#include <tuple>
#include <utility>

namespace qubus
{
namespace cuda
{

cuda_error::cuda_error(CUresult error_code_) : error_code_(error_code_)
{
}

CUresult cuda_error::error_code() const
{
    return error_code_;
}

const char* cuda_error::what() const noexcept
{
    const char* msg;

    check_cuda_error(cuGetErrorString(error_code(), &msg));

    return msg;
}

void check_cuda_error(CUresult result)
{
    if (result != CUDA_SUCCESS)
        throw cuda_error(result);
}

void init()
{
    check_cuda_error(cuInit(0));
}

int get_driver_version()
{
    int version;

    check_cuda_error(cuDriverGetVersion(&version));

    return version;
}

device::device(int ordinal)
{
    check_cuda_error(cuDeviceGet(&device_, ordinal));
}

device::device(device&& other) : device_(std::move(other.device_))
{
    other.device_ = 0;
}

device& device::operator=(device&& other)
{
    device_ = std::move(other.device_);
    other.device_ = 0;

    return *this;
}

std::string device::name() const
{
    std::vector<char> id(100);

    check_cuda_error(cuDeviceGetName(id.data(), id.size(), device_));

    return std::string(id.data());
}

std::size_t device::total_mem() const
{
    std::size_t value;

    check_cuda_error(cuDeviceTotalMem(&value, device_));

    return value;
}

compute_capability_info device::compute_capability() const
{
    int major_revision;
    int minor_revision;

    check_cuda_error(cuDeviceComputeCapability(&major_revision, &minor_revision, device_));

    return compute_capability_info(major_revision, minor_revision);
}

int device::get_attribute(CUdevice_attribute attribute) const
{
    int value;

    check_cuda_error(cuDeviceGetAttribute(&value, attribute, device_));

    return value;
}

int device::max_threads_per_block() const
{
    return get_attribute(CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK);
}

int device::max_shared_memory_per_block() const
{
    return get_attribute(CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK);
}

int device::warp_size() const
{
    return get_attribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE);
}

int device::max_block_dim_x() const
{
    return get_attribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X);
}

int device::max_grid_dim_x() const
{
    return get_attribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X);
}

CUdevice device::native_handle() const
{
    return device_;
}

int get_device_count()
{
    int count;

    check_cuda_error(cuDeviceGetCount(&count));

    return count;
}

std::vector<device> get_devices()
{
    auto count = get_device_count();

    std::vector<device> devices;
    devices.reserve(count);

    for (decltype(count) i = 0; i < count; ++i)
    {
        devices.push_back(device(i));
    }

    return devices;
}

context_view::context_view() : context_(0)
{
}

context_view::context_view(CUcontext handle_) noexcept : context_(handle_)
{
}

CUcontext context_view::native_handle() const
{
    return context_;
}

bool operator==(const context_view& lhs, const context_view& rhs)
{
    return lhs.native_handle() == rhs.native_handle();
}

bool operator!=(const context_view& lhs, const context_view& rhs)
{
    return !(lhs == rhs);
}

context::context() : context_(0)
{
}

context::context(const device& dev)
{
    check_cuda_error(cuCtxCreate(&context_, CU_CTX_SCHED_BLOCKING_SYNC, dev.native_handle()));

    CUcontext ctx;

    check_cuda_error(cuCtxPopCurrent(&ctx));
}

context::context(CUcontext handle_) noexcept : context_(handle_)
{
}

context::context(context&& other) : context_(std::move(other.context_))
{
    other.context_ = 0;
}

context& context::operator=(context&& other)
{
    context_ = std::move(other.context_);
    other.context_ = 0;

    return *this;
}

context::~context()
{
    if (context_ != 0)
    {
        cuCtxDestroy(context_);
    }
}

void context::activate() const
{
    check_cuda_error(cuCtxPushCurrent(this->native_handle()));
}

void context::deactivate() const
{
    CUcontext old_ctx;

    check_cuda_error(cuCtxPopCurrent(&old_ctx));
}

CUcontext context::native_handle() const
{
    return context_;
}

bool operator==(const context& lhs, const context& rhs)
{
    return lhs.native_handle() == rhs.native_handle();
}

bool operator!=(const context& lhs, const context& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const context_view& lhs, const context& rhs)
{
    return lhs.native_handle() == rhs.native_handle();
}

bool operator!=(const context_view& lhs, const context& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const context& lhs, const context_view& rhs)
{
    return lhs.native_handle() == rhs.native_handle();
}

bool operator!=(const context& lhs, const context_view& rhs)
{
    return !(lhs == rhs);
}

namespace this_context
{
context_view get()
{
    CUcontext current_ctx;

    check_cuda_error(cuCtxGetCurrent(&current_ctx));

    return context_view(current_ctx);
}

void synchronize()
{
    check_cuda_error(cuCtxSynchronize());
}
}

context_guard::context_guard(const context& ctx_)
: ctx_(&ctx_)
{
    this->ctx_->activate();
}

context_guard::~context_guard()
{
    deactivate();
}

context_guard::context_guard(context_guard&& other)
: ctx_(other.ctx_)
{
    other.ctx_ = nullptr;
}

context_guard& context_guard::operator=(context_guard&& other)
{
    ctx_ = other.ctx_;
    other.ctx_ = nullptr;

    return *this;
}

void context_guard::deactivate()
{
    if (ctx_)
    {
        ctx_->deactivate();
    }
}

namespace
{
void stream_callback_trampoline(CUstream this_stream, CUresult error_code, void* user_data)
{
    stream::callback_closure& callback_closure = *static_cast<stream::callback_closure*>(user_data);

    callback_closure();
}
}

stream::stream()
{
    check_cuda_error(cuStreamCreate(&stream_handle_, CU_STREAM_DEFAULT));
}

stream::stream(stream&& other)
: stream_handle_(std::move(other.stream_handle_)),
  pending_callbacks_(std::move(other.pending_callbacks_))
{
    other.stream_handle_ = 0;
}

stream& stream::operator=(stream&& other)
{
    stream_handle_ = std::move(other.stream_handle_);
    other.stream_handle_ = 0;
    pending_callbacks_ = std::move(other.pending_callbacks_);

    return *this;
}

stream::~stream()
{
    if (stream_handle_ != 0)
    {
        cuStreamSynchronize(stream_handle_);
        cuStreamDestroy(stream_handle_);
    }

    QUBUS_ASSERT(pending_callbacks_.empty(), "There are still pending callbacks.");
}

void stream::add_callback(std::function<void()> callback)
{
    auto new_pending_callback_position = pending_callbacks_.emplace(end(pending_callbacks_));
    *new_pending_callback_position = [=] {
        callback();
        pending_callbacks_.erase(new_pending_callback_position);
    };

    check_cuda_error(cuStreamAddCallback(stream_handle_, stream_callback_trampoline,
                                         &*new_pending_callback_position, 0));
}

CUstream stream::native_handle() const
{
    return stream_handle_;
}

bool operator==(const architecture_version& lhs, const architecture_version& rhs)
{
    return lhs.major_version == rhs.major_version && lhs.minor_version == rhs.minor_version;
}

bool operator!=(const architecture_version& lhs, const architecture_version& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const architecture_version& lhs, const architecture_version& rhs)
{
    return std::tie(lhs.major_version, lhs.minor_version) <
           std::tie(rhs.major_version, rhs.minor_version);
}

bool operator>(const architecture_version& lhs, const architecture_version& rhs)
{
    return !(lhs <= rhs);
}

bool operator<=(const architecture_version& lhs, const architecture_version& rhs)
{
    return lhs < rhs || lhs == rhs;
}

bool operator>=(const architecture_version& lhs, const architecture_version& rhs)
{
    return !(lhs < rhs);
}

function::function(CUfunction function_) : function_(function_)
{
}

int function::get_attribute(CUfunction_attribute attr) const
{
    int value;

    check_cuda_error(cuFuncGetAttribute(&value, attr, function_));

    return value;
}

int function::max_threads_per_block() const
{
    return get_attribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);
}

int function::shared_memory_size() const
{
    return get_attribute(CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES);
}

int function::number_of_register_per_thread() const
{
    return get_attribute(CU_FUNC_ATTRIBUTE_NUM_REGS);
}

architecture_version function::ptx_version() const
{
    int mangled_version = get_attribute(CU_FUNC_ATTRIBUTE_PTX_VERSION);

    int major_version = mangled_version / 10;
    int minor_version = mangled_version % 10;

    return architecture_version(major_version, minor_version);
}

architecture_version function::binary_version() const
{
    int mangled_version = get_attribute(CU_FUNC_ATTRIBUTE_BINARY_VERSION);

    int major_version = mangled_version / 10;
    int minor_version = mangled_version % 10;

    return architecture_version(major_version, minor_version);
}

void function::set_cache_config(CUfunc_cache config)
{
    check_cuda_error(cuFuncSetCacheConfig(function_, config));
}

void function::set_shared_memory_config(CUsharedconfig config)
{
    check_cuda_error(cuFuncSetSharedMemConfig(function_, config));
}

CUfunction function::native_handle() const
{
    return function_;
}

int calculate_max_active_blocks_per_multiprocessor(const function& kernel, int block_size,
                                                   std::size_t dynamic_shared_mem_size)
{
    int num_blocks;

    check_cuda_error(cuOccupancyMaxActiveBlocksPerMultiprocessor(
        &num_blocks, kernel.native_handle(), block_size, dynamic_shared_mem_size));

    return num_blocks;
}

suggested_launch_config
calculate_launch_config_with_max_occupancy(const function& kernel,
                                           std::size_t dynamic_shared_mem_size)
{
    int min_grid_size;
    int block_size;

    check_cuda_error(cuOccupancyMaxPotentialBlockSize(
        &min_grid_size, &block_size, kernel.native_handle(), 0, dynamic_shared_mem_size, 0));

    return suggested_launch_config(min_grid_size, block_size);
}

module::module(const std::string& ptx_code)
{
    static_assert(sizeof(std::uintptr_t) == sizeof(void*),
                  "sizeof(std::uintptr_t) == sizeof(void*) must hold");

    std::uintptr_t log_buffer_size = 8000;
    std::uintptr_t error_buffer_size = 8000;

    std::vector<char> log_buffer(log_buffer_size);
    std::vector<char> error_buffer(error_buffer_size);

    CUjit_option options[] = {CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES, CU_JIT_INFO_LOG_BUFFER,
                              CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES, CU_JIT_ERROR_LOG_BUFFER};

    void* options_values[] = {reinterpret_cast<void*>(log_buffer_size), log_buffer.data(),
                              reinterpret_cast<void*>(error_buffer_size), error_buffer.data()};

    check_cuda_error(cuModuleLoadDataEx(&module_, ptx_code.c_str(), 4, options, options_values));

    std::memcpy(&log_buffer_size, &options_values[0], sizeof(std::uintptr_t));
    std::memcpy(&error_buffer_size, &options_values[2], sizeof(std::uintptr_t));

    log_msg_ = std::string(log_buffer.data(), log_buffer_size);
    error_msg_ = std::string(error_buffer.data(), error_buffer_size);
}

module::module(module&& other) : module_(std::move(other.module_))
{
    other.module_ = 0;
}

module& module::operator=(module&& other)
{
    module_ = std::move(other.module_);
    other.module_ = 0;

    return *this;
}

module::~module()
{
    if (module_ != 0)
    {
        cuModuleUnload(module_);
    }
}

const std::string& module::log_message() const
{
    return log_msg_;
}

const std::string& module::error_message() const
{
    return error_msg_;
}

function module::get_function(const std::string& name) const
{
    CUfunction fn;
    check_cuda_error(cuModuleGetFunction(&fn, native_handle(), name.c_str()));

    return function(fn);
}

CUmodule module::native_handle() const
{
    return module_;
}

device_ptr device_malloc(std::size_t size)
{
    device_ptr ptr;

    check_cuda_error(cuMemAlloc(&ptr, size));

    return ptr;
}

void device_free(device_ptr ptr)
{
    if (ptr != 0)
    {
        check_cuda_error(cuMemFree(ptr));
    }
}

void memcpy(void* dst, device_ptr src, std::size_t size)
{
    check_cuda_error(cuMemcpyDtoH(dst, src, size));
}

void memcpy(device_ptr dst, void* src, std::size_t size)
{
    check_cuda_error(cuMemcpyHtoD(dst, src, size));
}

unique_device_ptr::unique_device_ptr(device_ptr ptr_) : ptr_(ptr_)
{
}

unique_device_ptr::unique_device_ptr(unique_device_ptr&& other) : ptr_(std::move(other.ptr_))
{
    other.ptr_ = 0;
}

unique_device_ptr& unique_device_ptr::operator=(unique_device_ptr&& other)
{
    ptr_ = std::move(other.ptr_);
    other.ptr_ = 0;

    return *this;
}

unique_device_ptr::~unique_device_ptr()
{
    try
    {
        device_free(ptr_);
    }
    catch (...)
    {
    }
}

device_ptr unique_device_ptr::get()
{
    return ptr_;
}
}
}
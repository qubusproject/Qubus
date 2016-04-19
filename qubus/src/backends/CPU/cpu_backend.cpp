#include <hpx/config.hpp>

#include <qbb/qubus/backends/cpu_backend.hpp>

#include <qbb/qubus/backend.hpp>
#include <qbb/qubus/host_backend.hpp>

#include <qbb/qubus/abi_info.hpp>
#include <qbb/qubus/metadata_builder.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/host_allocator.hpp>

#include <qbb/qubus/backends/cpu_compiler.hpp>

#include <qbb/qubus/backends/cpu_allocator.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/util/make_unique.hpp>

#include <qubus/qbb_qubus_export.h>

#include <hpx/async.hpp>

#include <hpx/include/lcos.hpp>

#include <qbb/qubus/hpx_utils.hpp>

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <qbb/util/optional_ref.hpp>
#include <qbb/util/make_unique.hpp>
#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <algorithm>
#include <vector>
#include <cstdlib>

namespace qbb
{
namespace qubus
{

class cpu_runtime
{
public:
    cpu_runtime(const void* address_translation_table_, std::size_t bucket_count_)
    : address_translation_table_(address_translation_table_), bucket_count_(bucket_count_),
      scratch_mem_(8 * 1024 * 1024), current_stack_ptr_(scratch_mem_.data())
    {
    }

    void* translate_address(address addr)
    {
        using bucket_type = std::pair<address, void*>;

        const bucket_type* buckets = static_cast<const bucket_type*>(address_translation_table_);

        auto bucket_count = bucket_count_;
        auto guessed_bucket_id = std::hash<address>()(addr) % bucket_count;
        auto bucket_id = guessed_bucket_id;

        do
        {
            auto& bucket = buckets[bucket_id];

            if (bucket.first == addr)
            {
                return bucket.second;
            }

            bucket_id = (bucket_id + 1) % bucket_count;
        } while (bucket_id != guessed_bucket_id);

        std::abort(); // invalid address
    }

    void* alloc_scratch_mem(util::index_t size)
    {
        void* addr = current_stack_ptr_;

        current_stack_ptr_ += size;

        return addr;
    }

    void dealloc_scratch_mem(util::index_t size)
    {
        current_stack_ptr_ -= size;
    }

private:
    const void* address_translation_table_;
    std::size_t bucket_count_;
    std::vector<char> scratch_mem_;
    char* current_stack_ptr_;
};

extern "C" QBB_QUBUS_EXPORT void* qbb_qubus_cpurt_translate_address(cpu_runtime* runtime,
                                                                    address* addr)
{
    return runtime->translate_address(*addr);
}

extern "C" QBB_QUBUS_EXPORT void* qbb_qubus_cpurt_alloc_scatch_mem(cpu_runtime* runtime,
                                                                   util::index_t size)
{
    return runtime->alloc_scratch_mem(size);
}

extern "C" QBB_QUBUS_EXPORT void qbb_qubus_cpurt_dealloc_scratch_mem(cpu_runtime* runtime,
                                                                     util::index_t size)
{
    runtime->dealloc_scratch_mem(size);
}

class caching_cpu_comiler
{
public:
    const cpu_plan& compile(const computelet& c) const
    {
        std::unique_lock<hpx::lcos::local::mutex> guard(cache_mutex_);

        auto pos = compilation_cache_.find(c.get_id().get_gid());

        if (pos != compilation_cache_.end())
        {
            return *pos->second;
        }
        else
        {
            auto compilation = underlying_compiler_.compile_computelet(c.code().get());

            pos = compilation_cache_.emplace(c.get_id().get_gid(), std::move(compilation)).first;

            return *pos->second;
        }
    }

private:
    mutable cpu_compiler underlying_compiler_; // FIXME: Make the cpmpiler non-mutable.
    mutable std::unordered_map<hpx::naming::gid_type, std::unique_ptr<cpu_plan>> compilation_cache_;
    mutable hpx::lcos::local::mutex cache_mutex_;
};

class cpu_vpu : public vpu_interface, public hpx::components::component_base<cpu_vpu>
{
public:
    cpu_vpu() = default;

    cpu_vpu(host_address_space* address_space_, abi_info abi_)
    : address_space_(address_space_), abi_(std::move(abi_))
    {
    }

    virtual ~cpu_vpu() = default;

    void execute(computelet c, execution_context ctx) const override
    {
        const auto& compilation = compiler_.compile(c);

        std::vector<address> task_args;

        std::vector<host_address_space::pin> pins;

        host_address_translation_table address_translation_table;

        for (const auto& arg : ctx.args())
        {
            auto pin = address_space_->resolve_object(arg).get().pin_object().get();

            task_args.push_back(pin.addr());

            address_translation_table.register_mapping(pin.addr(), pin.data().ptr());

            pins.push_back(std::move(pin));
        }

        hpx::apply([
            &compilation,
            task_args = std::move(task_args),
            pins = std::move(pins),
            address_translation_table = std::move(address_translation_table)
        ]
                          {
                              cpu_runtime runtime(address_translation_table.get_table(),
                                                  address_translation_table.bucket_count());

                              compilation.execute(task_args, runtime);
                          });
    }

private:
    caching_cpu_comiler compiler_;
    host_address_space* address_space_;
    abi_info abi_;
};

class cpu_backend final : public host_backend
{
public:
    cpu_backend(const abi_info& abi_)
    : address_space_(std::make_unique<host_allocator>()),
      compiler_(util::make_unique<cpu_compiler>())
    {
        // use hwloc to obtain informations over all local CPUs
        vpus_.push_back(new_here<cpu_vpu>(&address_space_, abi_));
    }

    virtual ~cpu_backend() = default;

    std::string id() const override
    {
        return "qubus.cpu";
    }

    std::vector<vpu> vpus() const
    {
        return vpus_;
    }

    host_address_space& get_host_address_space() override
    {
        return address_space_;
    }

private:
    host_address_space address_space_;
    std::unique_ptr<cpu_compiler> compiler_;
    std::vector<vpu> vpus_;
};

extern "C" QBB_QUBUS_EXPORT unsigned int cpu_backend_get_backend_type()
{
    return static_cast<unsigned int>(backend_type::host);
}

extern "C" QBB_QUBUS_EXPORT unsigned long int cpu_backend_get_api_version()
{
    return 0;
}

std::unique_ptr<cpu_backend> the_cpu_backend;
std::once_flag cpu_backend_init_flag;

extern "C" QBB_QUBUS_EXPORT backend* init_cpu_backend(const abi_info* abi)
{
    std::call_once(cpu_backend_init_flag, [&]
                   {
                       the_cpu_backend = util::make_unique<cpu_backend>(*abi);
                   });

    return the_cpu_backend.get();
}
}
}

using cpu_vpu_server_type = hpx::components::component<qbb::qubus::cpu_vpu>;
HPX_REGISTER_COMPONENT(cpu_vpu_server_type, qbb_qubus_cpu_vpu);

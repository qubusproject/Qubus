#include <iostream>

#include <qbb/kubus/type.hpp>
#include <qbb/kubus/object.hpp>
#include <qbb/kubus/memory_type.hpp>
#include <qbb/kubus/allocator.hpp>
#include <qbb/kubus/memory_block.hpp>
#include <qbb/kubus/managed_memory_block.hpp>

#include <qbb/util/multi_method.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>
#include <qbb/util/handle.hpp>

#include "cpu_memory_allocator.hpp"

#include <boost/thread/future.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <ostream>
#include <memory>
#include <cstring>
#include <utility>
#include <map>
#include <vector>
#include <functional>
#include <mutex>
#include <tuple>
#include <numeric>
#include <typeindex>

namespace qbb
{
namespace kubus
{

qbb::util::handle_factory object_handle_factory;

class mock_allocator
{
public:
    mock_allocator(allocator underlying_allocator_, std::size_t allocated_memory_,
                   std::size_t total_memory_)
    : underlying_allocator_{std::move(underlying_allocator_)}, allocated_memory_{allocated_memory_},
      total_memory_{total_memory_}
    {
    }

    memory_block allocate(std::size_t size)
    {
        if(allocated_memory_ + size < total_memory_)
        {
            allocated_memory_ += size;
            return underlying_allocator_.allocate(size);
        }
        else
        {
            return {};
        }
    }
    
    void deallocate(memory_block& mem_block)
    {
        std::size_t size = mem_block.size();
        
        underlying_allocator_.deallocate(mem_block);
        allocated_memory_ -= size;
    }
private:
    allocator underlying_allocator_;
    std::size_t allocated_memory_;
    std::size_t total_memory_;
};

class object_space;

class evicting_allocator
{
public:
    evicting_allocator(allocator underlying_allocator_, object_space* ospace_);

    memory_block allocate(std::size_t size);
    void deallocate(memory_block& mem_block);
private:
    allocator underlying_allocator_;
    object_space* ospace_;
};

class object_space
{
public:
    explicit object_space(memory_type mem_type_, allocator allocator_)
    : allocator_(std::move(allocator_), this), mem_type_(mem_type_)
    {
    }

    virtual ~object_space() = default;

    qbb::util::handle register_object(object new_object)
    {
        qbb::util::handle new_handle = object_handle_factory.create();

        objects_.insert(std::make_pair(new_handle, new_object));

        return new_handle;
    }

    // FIXME: return a future
    object get_object(const qbb::util::handle& object_handle) const
    {
        auto iter = objects_.find(object_handle);

        if (iter != objects_.end())
        {
            return iter->second;
        }
        else
        {
            object new_object = clone_object(object_handle);

            objects_.insert(std::make_pair(object_handle, new_object));

            return new_object;
        }
    }

    void register_fallback_space(object_space* fallback_space)
    {
        fallback_spaces_.push_back(fallback_space);
    }

    void dump() const
    {
        std::cout << "object table\n";
        std::cout << "contains " << objects_.size() << " objects\n";

        for (const auto& handle_object_pair : objects_)
        {
            std::cout << handle_object_pair.first << " -> " << handle_object_pair.second.ptr()
                      << "\n";
        }

        std::cout.flush();
    }

    bool evict_objects(std::size_t QBB_UNUSED(hint))
    {
        for (auto first = objects_.begin(); first != objects_.end(); ++first)
        {
            if (!first->second.is_pinned())
            {
                std::cout << "evicting object " << first->first << std::endl;
                
                objects_.erase(first);

                return true;
            }
        }

        return false;
    }
private:
    object clone_object(const qbb::util::handle& object_handle) const;

    mutable evicting_allocator allocator_;
    
    memory_type mem_type_;
    
    mutable std::map<qbb::util::handle, object>
    objects_; // FIXME: we need to protect this with a mutex
    std::vector<object_space*> fallback_spaces_;
};

evicting_allocator::evicting_allocator(allocator underlying_allocator_, object_space* ospace_)
: underlying_allocator_(underlying_allocator_), ospace_(ospace_)
{
}

memory_block evicting_allocator::allocate(std::size_t size)
{
    memory_block memblock = underlying_allocator_.allocate(size);

    while (!memblock)
    {
        if (!ospace_->evict_objects(0))
            return {};

        memblock = underlying_allocator_.allocate(size);
    }

    return memblock;
}

void evicting_allocator::deallocate(memory_block& mem_block)
{
    underlying_allocator_.deallocate(mem_block);
}

qbb::util::sparse_multi_method<object(const qbb::util::virtual_<object>&,
                                      const qbb::util::virtual_<memory_type>&, allocator_view)>
clone_object_dispatcher = {};

object object_space::clone_object(const qbb::util::handle& object_handle) const
{
    for (auto& fallback_space : fallback_spaces_)
    {
        try
        {
            auto cloned_object = fallback_space->get_object(object_handle);

            std::cout << "fetching object from fallback space" << std::endl;

            object new_object = clone_object_dispatcher(cloned_object, mem_type_, allocator_);

            return new_object;
        }
        catch (const int&)
        {
        }
    }

    throw 0; // error: object not available
}

class disk_tensor_object
{
public:
    explicit disk_tensor_object(managed_memory_block mem_block_) : mem_block_{std::make_shared<managed_memory_block>(std::move(mem_block_))}
    {
    }
    
    std::size_t size() const
    {
        return mem_block_->size();
    }

    void* ptr()
    {
        return mem_block_->underlying_memory_block().as<cpu_memory_block>().data();
    }
    
    const void* ptr() const
    {
        return mem_block_->underlying_memory_block().as<cpu_memory_block>().data();
    }
private:
    std::shared_ptr<managed_memory_block> mem_block_;
};

class gpu_tensor_object
{
public:
    explicit gpu_tensor_object(managed_memory_block mem_block_) : mem_block_{std::make_shared<managed_memory_block>(std::move(mem_block_))}
    {
    }
    
    std::size_t size() const
    {
        return mem_block_->size();
    }

    void* ptr()
    {
        return mem_block_->underlying_memory_block().as<cpu_memory_block>().data();
    }
    
    const void* ptr() const
    {
        return mem_block_->underlying_memory_block().as<cpu_memory_block>().data();
    }
private:
    std::shared_ptr<managed_memory_block> mem_block_;
};

class cpu_tensor_object
{
public:
    explicit cpu_tensor_object(managed_memory_block mem_block_) : mem_block_{std::make_shared<managed_memory_block>(std::move(mem_block_))}
    {
    }

    std::size_t size() const
    {
        return mem_block_->size();
    }

    void* ptr()
    {
        return mem_block_->underlying_memory_block().as<cpu_memory_block>().data();
    }
    
    const void* ptr() const
    {
        return mem_block_->underlying_memory_block().as<cpu_memory_block>().data();
    }
private:
    std::shared_ptr<managed_memory_block> mem_block_;
};

memory_allocator allocator;

object allocate_disk_tensor(const std::vector<qbb::util::index_t> shape)
{
    qbb::util::index_t number_of_elements =
        std::accumulate(begin(shape), end(shape), 1, std::multiplies<qbb::util::index_t>());

    auto mem_block = allocator.allocate(sizeof(double) * number_of_elements);
        
    if(!mem_block)
        throw std::bad_alloc();
    
    object new_object = disk_tensor_object(managed_memory_block(mem_block, allocator));

    return new_object;
}

object allocate_cpu_tensor(const std::vector<qbb::util::index_t> shape)
{
    qbb::util::index_t number_of_elements =
        std::accumulate(begin(shape), end(shape), 1, std::multiplies<qbb::util::index_t>());

    object new_object = cpu_tensor_object(managed_memory_block(allocator.allocate(sizeof(double) * number_of_elements), allocator));

    return new_object;
}

object clone_tensor_disk_to_cpu(const disk_tensor_object& other, const cpu_memory&,
                                allocator_view allocator)
{
    auto memory = allocator.allocate(other.size());
    
    if(!memory)
        throw std::bad_alloc();
    
    auto cpu_memory_view = memory.as<cpu_memory_block>();
    
    std::memcpy(cpu_memory_view.data(), other.ptr(), other.size());

    object new_object = cpu_tensor_object(managed_memory_block(memory, allocator));

    return new_object;
}

object clone_tensor_cpu_to_gpu(const cpu_tensor_object& other, const gpu_memory&,
                               allocator_view allocator)
{
    auto memory = allocator.allocate(other.size());
    auto cpu_memory_view = memory.as<cpu_memory_block>();

    std::memcpy(cpu_memory_view.data(), other.ptr(), other.size());

    object new_object = gpu_tensor_object(managed_memory_block(memory, allocator));

    return new_object;
}
}
}

using namespace qbb::kubus;

int main(int QBB_UNUSED(argc), char** QBB_UNUSED(argv))
{
    clone_object_dispatcher.add_specialization(clone_tensor_disk_to_cpu);
    clone_object_dispatcher.add_specialization(clone_tensor_cpu_to_gpu);

    object_space disk_space(disk_memory{}, mock_allocator(memory_allocator(), 1600, 10000));
    object_space cpu_space(cpu_memory{}, mock_allocator(memory_allocator(), 0, 1000));
    object_space gpu_space(gpu_memory{}, mock_allocator(memory_allocator(), 0, 1000));

    cpu_space.register_fallback_space(&disk_space);
    gpu_space.register_fallback_space(&cpu_space);

    qbb::util::handle obj_handle = disk_space.register_object(allocate_disk_tensor({10, 10}));
    qbb::util::handle obj_handle2 = disk_space.register_object(allocate_disk_tensor({10, 10}));

    disk_space.dump();
    std::cout << std::endl;

    {
    auto obj = cpu_space.get_object(obj_handle);

    cpu_space.dump();
    std::cout << std::endl;

    //gpu_space.get_object(obj_handle);

    std::cout << obj.ptr() << std::endl;
    
    }
    
    cpu_space.get_object(obj_handle2);
    
    cpu_space.dump();
    std::cout << std::endl;
    
    gpu_space.get_object(obj_handle);
    
    cpu_space.dump();
    std::cout << std::endl;
    
    gpu_space.dump();
    std::cout << std::endl;

    //qbb::kubus::type t = qbb::kubus::types::double_();

    return 0;
}

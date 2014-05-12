#include <iostream>

#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>
#include <qbb/util/handle.hpp>

#include "cpu_memory_allocator.hpp"
#include "memory_block.hpp"

#include <boost/thread/future.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/container/flat_map.hpp>

#include <ostream>
#include <memory>
#include <utility>
#include <map>
#include <vector>
#include <functional>
#include <mutex>
#include <tuple>
#include <numeric>
#include <typeindex>

class generic_ptr
{
public:
    template <typename T>
    explicit generic_ptr(T ptr_)
    : self_{new generic_ptr_wrapper<T>(ptr_)}
    {
    }

    generic_ptr(const generic_ptr& other) : self_{other.self_->clone()}
    {
    }

    generic_ptr(generic_ptr&&) = default;

    generic_ptr& operator=(const generic_ptr& other)
    {
        *this = generic_ptr(other);

        return *this;
    }

    generic_ptr& operator=(generic_ptr&&) = default;

    template <typename T>
    T as() const
    {
        if (typeid(*self_) == typeid(generic_ptr_wrapper<T>))
        {
            return static_cast<const generic_ptr_wrapper<T>*>(self_.get())->get();
        }
        else
        {
            throw 0;
        }
    }

    void dump(std::ostream& os) const
    {
        self_->dump(os);
    }

private:
    class generic_ptr_interface
    {
    public:
        generic_ptr_interface() = default;
        virtual ~generic_ptr_interface() = default;

        generic_ptr_interface(const generic_ptr_interface&) = delete;
        generic_ptr_interface& operator=(const generic_ptr_interface&) = delete;

        virtual std::unique_ptr<generic_ptr_interface> clone() const = 0;
        virtual void dump(std::ostream& os) const = 0;
    };

    template <typename T>
    class generic_ptr_wrapper final : public generic_ptr_interface
    {
    public:
        generic_ptr_wrapper(T wrapped_ptr_) : wrapped_ptr_{std::move(wrapped_ptr_)}
        {
        }

        virtual ~generic_ptr_wrapper() = default;

        T get() const
        {
            return wrapped_ptr_;
        }

        std::unique_ptr<generic_ptr_interface> clone() const override
        {
            return std::unique_ptr<generic_ptr_interface>{new generic_ptr_wrapper<T>(wrapped_ptr_)};
        }

        void dump(std::ostream& os) const override
        {
            os << wrapped_ptr_;
        }

    private:
        T wrapped_ptr_;
    };

    std::unique_ptr<generic_ptr_interface> self_;
};

std::ostream& operator<<(std::ostream& os, const generic_ptr& value)
{
    value.dump(os);

    return os;
}

class object
{
public:
    virtual ~object() = default;

    virtual std::size_t size() const = 0;

    virtual generic_ptr ptr() const = 0;
};

qbb::util::handle_factory object_handle_factory;

class object_space
{
public:
    virtual ~object_space() = default;

    qbb::util::handle register_object(std::shared_ptr<object> new_object)
    {
        qbb::util::handle new_handle = object_handle_factory.create();

        objects_.insert(std::make_pair(new_handle, new_object));

        return new_handle;
    }

    std::shared_ptr<object> get_object(const qbb::util::handle& object_handle) const
    {
        auto iter = objects_.find(object_handle);

        if (iter != objects_.end())
        {
            return iter->second;
        }
        else
        {
            std::shared_ptr<object> new_object = clone_object(object_handle);

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
            std::cout << handle_object_pair.first << " -> " << handle_object_pair.second->ptr()
                      << "\n";
        }

        std::cout.flush();
    }

    bool evict_objects(std::size_t QBB_UNUSED(hint))
    {
        for(auto first = objects_.begin(); first != objects_.end(); ++first)
        {
            if(first->second.unique())
            {
                objects_.erase(first);
                
                return true;
            }
        }
        
        return false;
    }
private:
    std::shared_ptr<object> clone_object(const qbb::util::handle& object_handle) const;

    mutable std::map<qbb::util::handle, std::shared_ptr<object>>
    objects_; // FIXME: we need to protect this with a mutex
    std::vector<object_space*> fallback_spaces_;
};

class evicting_allocator
{
public:
    evicting_allocator(memory_allocator underlying_alloactor_, object_space* ospace_)
    : underlying_alloactor_(underlying_alloactor_), ospace_(ospace_)
    {
    }
    
    std::unique_ptr<cpu_memory_block> allocate(std::size_t size)
    {
        std::unique_ptr<cpu_memory_block> memblock = underlying_alloactor_.allocate(size);
        
        while(!memblock)
        {
            if(!ospace_->evict_objects(0))
                return nullptr;
            
            memblock = underlying_alloactor_.allocate(size);
        }
        
        return memblock;
    }
private:
    memory_allocator underlying_alloactor_;
    object_space* ospace_;
};

class clone_multimethod
{
public:
    using specialization_t = std::function<std::shared_ptr<object>(std::shared_ptr<object>)>;
    
    std::shared_ptr<object> operator()(std::type_index type, std::shared_ptr<object> cloned_object)
    {
        return nullptr;
    }
    
private:
    boost::container::flat_map<std::type_index, specialization_t> specialization_;
};

clone_multimethod clone_object = {}; 

std::shared_ptr<object> object_space::clone_object(const qbb::util::handle& object_handle) const
{
    for (auto& fallback_space : fallback_spaces_)
    {
        try
        {
            auto cloned_object = fallback_space->get_object(object_handle);

            std::cout << "fetching object from fallback space" << std::endl;

            //memory_allocator allocator;

            //auto new_object = std::make_shared<cpu_tensor_object>(allocator.allocate(10));

            // clone object here

            std::shared_ptr<object> new_object = ::clone_object(typeid(object), cloned_object);
            
            return new_object;
        }
        catch (const int&)
        {
        }
    }

    throw 0; // error: object not available
}

class tensor_object : public object
{
public:
    virtual ~tensor_object() = default;
};

class sparse_tensor_object : public object
{
public:
    virtual ~sparse_tensor_object() = default;
};

class cpu_tensor_object final : public tensor_object
{
public:
    explicit cpu_tensor_object(std::unique_ptr<cpu_memory_block> mem_block_)
    : mem_block_{std::move(mem_block_)}
    {
    }

    virtual ~cpu_tensor_object() = default;

    std::size_t size() const override
    {
        return mem_block_->size();
    }

    generic_ptr ptr() const override
    {
        return generic_ptr{mem_block_->data()};
    }

private:
    std::unique_ptr<cpu_memory_block> mem_block_;
};

std::shared_ptr<object> allocate_cpu_tensor(const std::vector<qbb::util::index_t> shape)
{
    memory_allocator allocator;

    qbb::util::index_t number_of_elements = std::accumulate(begin(shape), end(shape), 1, std::multiplies<qbb::util::index_t>());
    
    auto new_object = std::make_shared<cpu_tensor_object>(allocator.allocate(sizeof(double)*number_of_elements));
    
    return new_object;
}

int main(int QBB_UNUSED(argc), char** QBB_UNUSED(argv))
{
    object_space disk_space;
    object_space cpu_space;
    object_space gpu_space;

    cpu_space.register_fallback_space(&disk_space);
    gpu_space.register_fallback_space(&cpu_space);

    qbb::util::handle obj_handle = disk_space.register_object(allocate_cpu_tensor({10, 10}));
    
    disk_space.dump();
    std::cout << std::endl;

    auto obj = cpu_space.get_object(obj_handle);

    cpu_space.dump();
    std::cout << std::endl;
    
    std::cout << obj->ptr() << std::endl;

    return 0;
}

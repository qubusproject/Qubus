#include <qbb/kubus/local_address_space.hpp>

#include <qbb/kubus/logging.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/unused.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

local_address_space::local_address_space(std::unique_ptr<allocator> allocator_)
: allocator_(util::make_unique<evicting_allocator>(std::move(allocator_), this))
{
}

void local_address_space::register_mem_block(qbb::util::handle address, std::unique_ptr<memory_block> mem_block)
{
    objects_.emplace(address, std::move(mem_block));
}

// FIXME: return a future
std::shared_ptr<memory_block> local_address_space::get_mem_block(const qbb::util::handle& address) const
{
    auto iter = objects_.find(address);

    if (iter != objects_.end())
    {
        return iter->second;
    }
    else
    {
        //TODO: reimplement this
        /*object new_object = clone_object(object_handle);

        objects_.insert(std::make_pair(object_handle, new_object));

        return new_object;*/
        
        throw 0;
    }
}

allocator& local_address_space::get_allocator() const
{
    return *allocator_;
}

void local_address_space::dump() const
{
    BOOST_LOG_NAMED_SCOPE("local_address_space");
    
    logger slg;
    
    BOOST_LOG_SEV(slg, info) << "object table\n";
    BOOST_LOG_SEV(slg, info) << "contains " << objects_.size() << " objects\n";

    for (const auto& handle_object_pair : objects_)
    {
        BOOST_LOG_SEV(slg, info) << handle_object_pair.first << " -> " << handle_object_pair.second->ptr() << "\n";
    }
}

bool local_address_space::evict_objects(std::size_t QBB_UNUSED(hint))
{
    //TODO: protect this with a mutex
    //TODO: Don't evict anything if we don't have a valid fallback space.
    
    BOOST_LOG_NAMED_SCOPE("local_address_space");
    
    logger slg;
    
    for (auto first = objects_.begin(); first != objects_.end(); ++first)
    {
        if (/*!first->second.is_pinned()*/ first->second.unique())
        {
            BOOST_LOG_SEV(slg, info) << "evicting object " << first->first;

            objects_.erase(first);

            return true;
        }
    }

    return false;
}

/*object object_space::clone_object(const qbb::util::handle& object_handle) const
{
    for (auto& fallback_space : fallback_spaces_)
    {
        try
        {
            auto cloned_object = fallback_space->get_object(object_handle);

            std::cout << "fetching object from fallback space" << std::endl;

            //object new_object = clone_object_dispatcher(cloned_object, mem_type_, allocator_);

            //return new_object;
        }
        catch (const int&)
        {
            // Error handling for the 'throw 0' below !!!
            // We don't want to propagate the error until we
            // have tried all fallback spaces.
            // Note: This implementation is not *really* optimal.
        }
    }

    throw 0; // error: object not available (We shouldn't propagate this via exceptions.)
}*/

}
}
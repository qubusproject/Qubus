#include <qbb/qubus/object_factory.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/integers.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
 
void object_factory::add_factory_and_addr_space(const std::string& id, local_object_factory& factory,
                                                local_address_space& address_space)
{
    local_factories_[id] = &factory;
    address_spaces_[id] = &address_space;
}
    
std::unique_ptr<local_array> object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    //TODO: Reject arrays with a rank of 0
    
    auto factory = local_factories_.at("qubus.cpu");
    
    auto mem_block = factory->create_array(std::move(value_type), std::move(shape));
    
    auto address = handle_fac_.create();
    
    auto& address_space =  address_spaces_.at("qubus.cpu");
    
    address_space->register_mem_block(address, std::move(mem_block));

    return util::make_unique<local_array>(address, value_type, shape);
}

std::unique_ptr<local_tensor> object_factory::create_tensor(type value_type, std::vector<util::index_t> shape)
{
    return util::make_unique<local_tensor>(create_array(value_type, shape));
}
    
}
}
#ifndef QBB_KUBUS_OBJECT_FACTORY_HPP
#define QBB_KUBUS_OBJECT_FACTORY_HPP

#include <qbb/kubus/local_object_factory.hpp>
#include <qbb/kubus/local_address_space.hpp>

#include <qbb/kubus/local_array.hpp>
#include <qbb/kubus/local_tensor.hpp>

#include <qbb/kubus/IR/type.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/handle.hpp>

#include <vector>
#include <map>
#include <memory>

namespace qbb
{
namespace qubus
{

class object_factory
{
public:
    void add_factory_and_addr_space(const std::string& id, local_object_factory& factory,
                                    local_address_space& address_space);

    std::unique_ptr<local_array> create_array(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<local_tensor> create_tensor(type value_type, std::vector<util::index_t> shape);

private:
    util::handle_factory handle_fac_;
    std::map<std::string, local_object_factory*> local_factories_;
    std::map<std::string, local_address_space*> address_spaces_;
};
}
}

#endif
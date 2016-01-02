#ifndef QBB_QUBUS_OBJECT_FACTORY_HPP
#define QBB_QUBUS_OBJECT_FACTORY_HPP
#include <hpx/config.hpp>

#include <qbb/qubus/local_object_factory.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/local_array.hpp>
#include <qbb/qubus/local_tensor.hpp>
#include <qbb/qubus/struct.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/handle.hpp>

#include <vector>
#include <map>
#include <memory>

namespace qbb
{
namespace qubus
{

struct sparse_tensor_layout
{
explicit sparse_tensor_layout(util::index_t num_chunks, util::index_t nnz)
: num_chunks(num_chunks), nnz(nnz)
{
}

util::index_t num_chunks;
util::index_t nnz;
};

class object_factory
{
public:
    void add_factory_and_addr_space(const std::string& id, local_object_factory& factory,
                                    local_address_space& address_space);

    std::unique_ptr<local_array> create_array(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<local_tensor> create_tensor(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<struct_> create_struct(type struct_type,
                                           std::vector<std::unique_ptr<object>> members);

    std::unique_ptr<struct_> create_sparse_tensor(type value_type,
                                                        std::vector<util::index_t> shape,
                                                        const sparse_tensor_layout& layout);

private:
    util::handle_factory handle_fac_;
    std::map<std::string, local_object_factory*> local_factories_;
    std::map<std::string, local_address_space*> address_spaces_;
};
}
}

#endif
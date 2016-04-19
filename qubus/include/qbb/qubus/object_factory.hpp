#ifndef QBB_QUBUS_OBJECT_FACTORY_HPP
#define QBB_QUBUS_OBJECT_FACTORY_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/local_runtime.hpp>
#include <qbb/qubus/local_object_factory.hpp>

#include <qbb/qubus/abi_info.hpp>

#include <qbb/qubus/object.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/handle.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/include/components.hpp>

#include <vector>
#include <map>

namespace qbb
{
namespace qubus
{

struct sparse_tensor_layout
{
    sparse_tensor_layout() = default;

    explicit sparse_tensor_layout(util::index_t num_chunks, util::index_t nnz)
    : num_chunks(num_chunks), nnz(nnz)
    {
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & num_chunks;
        ar & nnz;
    }

    util::index_t num_chunks;
    util::index_t nnz;
};

class object_factory_server : public hpx::components::component_base<object_factory_server>
{
public:
    object_factory_server() = default;
    object_factory_server(abi_info abi_, std::vector<local_runtime> local_runtimes_);

    /*std::unique_ptr<local_array> create_array(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<local_tensor> create_tensor(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<struct_> create_struct(type struct_type,
                                           std::vector<std::unique_ptr<object>> members);

    std::unique_ptr<struct_> create_sparse_tensor(type value_type, std::vector<util::index_t> shape,
                                                  const sparse_tensor_layout& layout);*/

    object_client create_array(type value_type, std::vector<util::index_t> shape);

    HPX_DEFINE_COMPONENT_ACTION(object_factory_server, create_array, create_array_action);
private:
    abi_info abi_;
    std::vector<local_object_factory> local_factories_;
};

class object_factory : public hpx::components::client_base<object_factory, object_factory_server>
{
public:
    using base_type = hpx::components::client_base<object_factory, object_factory_server>;

    object_factory() = default;
    object_factory(hpx::future<hpx::id_type>&& id);

    object_client create_array(type value_type, std::vector<util::index_t> shape);
};

}
}

#endif
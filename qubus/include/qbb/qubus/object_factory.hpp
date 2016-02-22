#ifndef QBB_QUBUS_OBJECT_FACTORY_HPP
#define QBB_QUBUS_OBJECT_FACTORY_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/abi_info.hpp>
#include <qbb/qubus/allocator.hpp>

#include <qbb/qubus/local_array.hpp>
#include <qbb/qubus/local_tensor.hpp>
#include <qbb/qubus/struct.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/handle.hpp>
#include <qbb/util/unused.hpp>

#include <vector>
#include <map>
#include <memory>

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

class object_factory
{
public:
    explicit object_factory(abi_info abi_);

    std::unique_ptr<local_array> create_array(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<local_tensor> create_tensor(type value_type, std::vector<util::index_t> shape);
    std::unique_ptr<struct_> create_struct(type struct_type,
                                           std::vector<std::unique_ptr<object>> members);

    std::unique_ptr<struct_> create_sparse_tensor(type value_type, std::vector<util::index_t> shape,
                                                  const sparse_tensor_layout& layout);

private:
    abi_info abi_;
    std::unique_ptr<allocator> allocator_;
};
}
}

#endif
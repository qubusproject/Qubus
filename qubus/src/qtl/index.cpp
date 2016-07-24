#include <qbb/qubus/qtl/index.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::qtl::ast::id_type_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_qtl_ast_id_type_server);

namespace qbb
{
namespace qubus
{
namespace qtl
{
namespace ast
{

id_type::id_type(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

}
}
}
}

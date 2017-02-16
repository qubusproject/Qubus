#include <qubus/architecture_identifier.hpp>

namespace qubus
{

util::implementation_table architecture_identifier::implementation_table_ = {};

util::multi_method<bool(const util::virtual_<architecture_identifier>&,
                        const util::virtual_<architecture_identifier>&)>
is_same_architecture([](const architecture_identifier&, const architecture_identifier&)
                     {
                         return false;
                     });

namespace
{
bool is_same_architecture_host(const host_architecture_identifier&, const host_architecture_identifier&)
{
    return true;
}
}

QUBUS_DEFINE_MULTI_METHOD_SPECIALIZATION(is_same_architecture, is_same_architecture_host);

}

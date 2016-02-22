#include <qbb/qubus/object.hpp>

namespace qbb
{
namespace qubus
{

basic_object::basic_object() : last_modification_(hpx::make_ready_future())
{
}

basic_object::~basic_object()
{
}

hpx::shared_future<void> basic_object::get_last_modification() const
{
    return last_modification_;
}

void basic_object::record_modification(hpx::shared_future<void> modification)
{
    last_modification_ = modification;
}
}
}
#include <qbb/kubus/object.hpp>

namespace qbb
{
namespace kubus
{

void basic_object::on_destruction(const object_predestructor_signal::slot_type& subscriber) const
{
    on_destruction_.connect(subscriber);
}

void basic_object::destruct() const
{
    on_destruction_(*this);
}

}
}
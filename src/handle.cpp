#include <qbb/util/handle.hpp>

namespace qbb
{
namespace util
{

handle::handle() : id_{std::make_shared<char>()}
{
}

bool operator==(const handle& lhs, const handle& rhs)
{
    return lhs.id_ == rhs.id_;
}

bool operator<(const handle& lhs, const handle& rhs)
{
    return lhs.id_ < rhs.id_;
}

std::ostream& operator<<(std::ostream& os, const handle& value)
{
    os << static_cast<void*>(value.id_.get());

    return os;
}

handle handle_factory::create() const
{
    return handle();
}

}
}

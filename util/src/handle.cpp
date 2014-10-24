#include <qbb/util/handle.hpp>

#include <ios>

namespace qbb
{
namespace util
{

handle::handle(std::size_t id_) : id_{id_}
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
    os << std::hex << value.id_;

    return os;
}

constexpr std::size_t bucket_size = 32;

handle handle_factory::create() const
{
    return handle(next_free_id_++);
}

void handle_factory::release(handle) const
{
}

}
}

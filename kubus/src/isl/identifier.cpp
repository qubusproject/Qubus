#include <qbb/kubus/isl/identifier.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

    identifier::identifier(isl_id* handle_) : handle_{handle_}
    {
    }

    identifier::identifier(const identifier& other) : handle_{isl_id_copy(other.native_handle())}
    {
    }

    identifier::~identifier()
    {
        isl_id_free(handle_);
    }

    isl_id* identifier::native_handle() const
    {
        return handle_;
    }

    isl_id* identifier::release() noexcept
    {
        isl_id* temp = handle_;

        handle_ = nullptr;

        return temp;
    }

}
}
}
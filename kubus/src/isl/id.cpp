#include <qbb/kubus/isl/id.hpp>

#include <qbb/kubus/isl/context.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace isl
{

namespace
{
extern "C" {

void user_free(void* user) noexcept
{
    auto user_typed = static_cast<boost::any*>(user);

    delete user_typed;
}
}
}

id::id(isl_id* handle_) : handle_(handle_)
{
}

id::id(const context& ctx, const std::string& name)
: handle_(isl_id_alloc(ctx.native_handle(), name.c_str(), nullptr))
{
}

id::id(const context& ctx, const std::string& name, boost::any user_data)
: handle_(isl_id_alloc(ctx.native_handle(), name.c_str(), new boost::any(std::move(user_data))))
{
    isl_id_set_free_user(handle_, user_free);
}

id::id(const id& other) : handle_(isl_id_copy(other.native_handle()))
{
}

id::~id()
{
    isl_id_free(handle_);
}

isl_id* id::native_handle() const
{
    return handle_;
}

isl_id* id::release() noexcept
{
    isl_id* temp = handle_;

    handle_ = nullptr;

    return temp;
}

std::string id::name() const
{
    return std::string(isl_id_get_name(handle_));
}

const boost::any& id::user_data() const
{
    void* user = isl_id_get_user(handle_);

    auto user_typed = static_cast<boost::any*>(user);

    return *user_typed;
}

boost::any& id::user_data()
{
    void* user = isl_id_get_user(handle_);

    auto user_typed = static_cast<boost::any*>(user);

    return *user_typed;
}
}
}
}
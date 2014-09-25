#include <qbb/kubus/isl/set.hpp>

#include <qbb/kubus/isl/context.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

basic_set::basic_set(isl_basic_set* handle_) : handle_{handle_}
{
}

basic_set::basic_set(const context& ctx_, const std::string& desc_)
: handle_{isl_basic_set_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

basic_set::basic_set(const basic_set& other) : handle_{isl_basic_set_copy(other.native_handle())}
{
}

basic_set::~basic_set()
{
    isl_basic_set_free(handle_);
}

isl_basic_set* basic_set::native_handle() const
{
    return handle_;
}

isl_basic_set* basic_set::release() noexcept
{
    isl_basic_set* temp = handle_;

    handle_ = nullptr;

    return temp;
}

void basic_set::set_tuple_name(const std::string& name)
{
    handle_ = isl_basic_set_set_tuple_name(handle_, name.c_str());
}

set::set(isl_set* handle_) : handle_{handle_}
{
}

set::set(basic_set other) : handle_{isl_set_from_basic_set(other.release())}
{
}

set::set(const set& other) : handle_{isl_set_copy(other.native_handle())}
{
}

set::set(const context& ctx_, const std::string& desc_)
: handle_{isl_set_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

set::~set()
{
    isl_set_free(handle_);
}

isl_set* set::native_handle() const
{
    return handle_;
}

isl_set* set::release() noexcept
{
    isl_set* temp = handle_;

    handle_ = nullptr;

    return temp;
}

set& set::operator=(const set& other)
{
    isl_set_free(handle_);
    
    handle_ = isl_set_copy(other.handle_);
    
    return *this;
}

void set::add_constraint(constraint c)
{
    handle_ = isl_set_add_constraint(handle_, c.release());
}

space set::get_space() const
{
    return space(isl_set_get_space(handle_));
}

void set::set_tuple_name(const std::string& name)
{
    handle_ = isl_set_set_tuple_name(handle_, name.c_str());
}

set set::universe(space s)
{
    return set(isl_set_universe(s.release()));
}

set intersect(set lhs, set rhs)
{
    return set(isl_set_intersect(lhs.release(), rhs.release()));
}

set flat_product(set lhs, set rhs)
{
    return set(isl_set_flat_product(lhs.release(), rhs.release()));
}

set align_params(set s, space model)
{
    return set(isl_set_align_params(s.release(), model.release()));
}

union_set::union_set(isl_union_set* handle_) : handle_{handle_}
{
}

union_set::union_set(basic_set other) : handle_{isl_union_set_from_basic_set(other.release())}
{
}

union_set::union_set(set other) : handle_{isl_union_set_from_set(other.release())}
{
}

union_set::union_set(const union_set& other) : handle_{isl_union_set_copy(other.native_handle())}
{
}

union_set::union_set(const context& ctx_, const std::string& desc_)
: handle_{isl_union_set_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

union_set::~union_set()
{
    isl_union_set_free(handle_);
}

union_set& union_set::operator=(const union_set& other)
{
    isl_union_set_free(handle_);
    
    handle_ = isl_union_set_copy(other.handle_);
    
    return *this;
}

isl_union_set* union_set::native_handle() const
{
    return handle_;
}

isl_union_set* union_set::release() noexcept
{
    isl_union_set* temp = handle_;

    handle_ = nullptr;

    return temp;
}

union_set union_set::empty(space s)
{
    return union_set(isl_union_set_empty(s.release()));
}

union_set union_(union_set lhs, union_set rhs)
{
    return union_set(isl_union_set_union(lhs.release(), rhs.release()));
}

}
}
}
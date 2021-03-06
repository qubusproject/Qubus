#include <qubus/isl/set.hpp>

#include <qubus/isl/context.hpp>
#include <qubus/isl/pw_multi_aff.hpp>

namespace qubus
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

basic_set::basic_set(basic_set&& other) noexcept : handle_(other.release())
{
}

basic_set::~basic_set()
{
    if (handle_ != nullptr)
    {
        isl_basic_set_free(handle_);
    }
}

basic_set& basic_set::operator=(const basic_set& other)
{
    if (handle_ != nullptr)
    {
        isl_basic_set_free(handle_);
    }

    handle_ = isl_basic_set_copy(other.native_handle());

    return *this;
}

basic_set& basic_set::operator=(basic_set&& other) noexcept
{
    if (handle_ != nullptr)
    {
        isl_basic_set_free(handle_);
    }

    handle_ = other.release();

    return *this;
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

void basic_set::add_constraint(constraint c)
{
    handle_ = isl_basic_set_add_constraint(handle_, c.release());
}

space basic_set::get_space() const
{
    return space(isl_basic_set_get_space(handle_));
}

void basic_set::set_tuple_name(const std::string& name)
{
    handle_ = isl_basic_set_set_tuple_name(handle_, name.c_str());
}

basic_set basic_set::universe(space s)
{
    return basic_set(isl_basic_set_universe(s.release()));
}

bool operator==(const basic_set& lhs, const basic_set& rhs)
{
    return isl_basic_set_is_equal(lhs.native_handle(), rhs.native_handle()) != 0;
}

bool operator!=(const basic_set& lhs, const basic_set& rhs)
{
    return !(lhs == rhs);
}

bool is_subset(const basic_set& lhs, const basic_set& rhs)
{
    return isl_basic_set_is_subset(lhs.native_handle(), rhs.native_handle()) != 0;
}

bool is_empty(const basic_set& s)
{
    return isl_basic_set_is_empty(s.native_handle()) != 0;
}

basic_set flat_product(basic_set lhs, basic_set rhs)
{
    return basic_set(isl_basic_set_flat_product(lhs.release(), rhs.release()));
}

basic_set align_params(basic_set s, space model)
{
    return basic_set(isl_basic_set_align_params(s.release(), model.release()));
}

basic_set add_dims(basic_set s, isl_dim_type type, unsigned int n)
{
    return basic_set(isl_basic_set_add_dims(s.release(), type, n));
}

basic_set project_out(basic_set s, isl_dim_type type, unsigned int first, unsigned int n)
{
    return basic_set(isl_basic_set_project_out(s.release(), type, first, n));
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

set::set(set&& other) noexcept
: handle_(other.release())
{
}

set::set(const context& ctx_, const std::string& desc_)
: handle_{isl_set_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

set::~set()
{
    if (handle_ != nullptr)
    {
        isl_set_free(handle_);
    }
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
    if (handle_ != nullptr)
    {
        isl_set_free(handle_);
    }
    
    handle_ = isl_set_copy(other.handle_);
    
    return *this;
}

set& set::operator=(set&& other) noexcept
{
    if (handle_ != nullptr)
    {
        isl_set_free(handle_);
    }

    handle_ = other.release();

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

int set::dim(isl_dim_type type) const
{
    return isl_set_dim(handle_, type);
}

void set::set_tuple_name(const std::string& name)
{
    handle_ = isl_set_set_tuple_name(handle_, name.c_str());
}

std::string set::get_tuple_name() const
{
    return std::string(isl_set_get_tuple_name(handle_));
}

void set::set_dim_name(isl_dim_type type, int pos, const std::string& name)
{
    handle_ = isl_set_set_dim_name(handle_, type, pos, name.c_str());
}

boost::string_ref set::get_dim_name(isl_dim_type type, int pos) const
{
    return isl_set_get_dim_name(handle_, type, pos);
}

bool set::bounded() const
{
    return isl_set_is_bounded(handle_) != 0;
}

set set::universe(space s)
{
    return set(isl_set_universe(s.release()));
}

set set::empty(space s)
{
    return set(isl_set_empty(s.release()));
}

set union_(set lhs, set rhs)
{
    return set(isl_set_union(lhs.release(), rhs.release()));
}

set intersect(set lhs, set rhs)
{
    return set(isl_set_intersect(lhs.release(), rhs.release()));
}

set intersect_params(set lhs, set rhs)
{
    return set(isl_set_intersect_params(lhs.release(), rhs.release())); 
}

set substract(set lhs, set rhs)
{
    return set(isl_set_subtract(lhs.release(), rhs.release()));
}

set complement(set arg)
{
    return set(isl_set_complement(arg.release()));
}

bool operator==(const set& lhs, const set& rhs)
{
    return isl_set_is_equal(lhs.native_handle(), rhs.native_handle()) != 0;
}

bool operator!=(const set& lhs, const set& rhs)
{
    return !(lhs == rhs);
}

bool is_subset(const set& lhs, const set& rhs)
{
    return isl_set_is_subset(lhs.native_handle(), rhs.native_handle()) != 0;
}

bool is_strict_subset(const set& lhs, const set& rhs)
{
    return isl_set_is_strict_subset(lhs.native_handle(), rhs.native_handle()) != 0;
}

bool is_empty(const set& s)
{
    return isl_set_is_empty(s.native_handle()) != 0;
}

set get_params(set s)
{
    return set(isl_set_params(s.release()));
}

set flat_product(set lhs, set rhs)
{
    return set(isl_set_flat_product(lhs.release(), rhs.release()));
}

set align_params(set s, space model)
{
    return set(isl_set_align_params(s.release(), model.release()));
}

set project_out(set s, isl_dim_type type, unsigned int first, unsigned int n)
{
    return set(isl_set_project_out(s.release(), type, first, n));
}

set lexmin(set s)
{
    return set(isl_set_lexmin(s.release()));
}

set lexmax(set s)
{
    return set(isl_set_lexmax(s.release()));
}

set add_dims(set s, isl_dim_type type, unsigned int n)
{
    return set(isl_set_add_dims(s.release(), type, n));
}

set coalesce(set s)
{
    return set(isl_set_coalesce(s.release()));
}

set detect_equalities(set s)
{
    return set(isl_set_detect_equalities(s.release()));
}

set remove_redundancies(set s)
{
    return set(isl_set_remove_redundancies(s.release()));
}

set simplify(set s)
{
    return remove_redundancies(detect_equalities(coalesce(std::move(s))));
}

union_set::union_set(isl_union_set* handle_) : handle_(handle_)
{
}

union_set::union_set(basic_set other) : handle_{isl_union_set_from_basic_set(other.release())}
{
}

union_set::union_set(set other) : handle_{isl_union_set_from_set(other.release())}
{
}

union_set::union_set(const union_set& other) : handle_(isl_union_set_copy(other.native_handle()))
{
}

union_set::union_set(union_set&& other) noexcept : handle_(other.release())
{
}

union_set::union_set(const context& ctx_, const std::string& desc_)
: handle_{isl_union_set_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

union_set::~union_set()
{
    if (handle_ != nullptr)
    {
        isl_union_set_free(handle_);
    }
}

union_set& union_set::operator=(const union_set& other)
{
    if (handle_ != nullptr)
    {
        isl_union_set_free(handle_);
    }
    
    handle_ = isl_union_set_copy(other.handle_);
    
    return *this;
}

union_set& union_set::operator=(union_set&& other) noexcept
{
    if (handle_ != nullptr)
    {
        isl_union_set_free(handle_);
    }

    handle_ = other.release();

    return *this;
}

namespace
{
extern "C" isl_stat add_set(isl_set* s, void* user) noexcept;
}

std::vector<set> union_set::get_sets() const
{
    std::vector<set> result;

    isl_union_set_foreach_set(handle_, add_set, &result);

    return result;
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

union_set intersect(union_set lhs, union_set rhs)
{
    return union_set(isl_union_set_intersect(lhs.release(), rhs.release()));
}

bool is_subset(const union_set& lhs, const union_set& rhs)
{
    return isl_union_set_is_subset(lhs.native_handle(), rhs.native_handle()) != 0;
}

bool is_strict_subset(const union_set& lhs, const union_set& rhs)
{
    return isl_union_set_is_strict_subset(lhs.native_handle(), rhs.native_handle())  != 0;
}

bool operator==(const union_set& lhs, const union_set& rhs)
{
    return isl_union_set_is_equal(lhs.native_handle(), rhs.native_handle())  != 0;
}

bool operator!=(const union_set& lhs, const union_set& rhs)
{
    return !(lhs == rhs);
}

bool is_empty(const union_set& s)
{
    return isl_union_set_is_empty(s.native_handle())  != 0;
}

union_set add_set(union_set uset, set s)
{
    return union_set(isl_union_set_add_set(uset.release(), s.release()));
}

set extract_set(const union_set& uset, space s)
{
    return set(isl_union_set_extract_set(uset.native_handle(), s.release()));
}

union_set coalesce(union_set s)
{
    return union_set(isl_union_set_coalesce(s.release()));
}

union_set detect_equalities(union_set s)
{
    return union_set(isl_union_set_detect_equalities(s.release()));
}

union_set remove_redundancies(union_set s)
{
    return union_set(isl_union_set_remove_redundancies(s.release()));
}

union_set simplify(union_set s)
{
    return remove_redundancies(detect_equalities(coalesce(std::move(s))));
}

namespace
{
extern "C" {

isl_stat add_set(isl_set* s, void* user) noexcept
{
    auto& sets = *static_cast<std::vector<set>*>(user);

    sets.emplace_back(s);

    return isl_stat_ok;
}
}
}

}
}
#include <qbb/kubus/isl/map.hpp>

#include <qbb/kubus/isl/context.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

basic_map::basic_map(isl_basic_map* handle_) : handle_{handle_}
{
}

basic_map::basic_map(const basic_map& other) : handle_{isl_basic_map_copy(other.native_handle())}
{
}

basic_map::basic_map(basic_map&& other) noexcept : handle_{other.release()}
{
}

basic_map::basic_map(const context& ctx_, const std::string& desc_)
: handle_{isl_basic_map_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

basic_map& basic_map::operator=(basic_map other)
{
    isl_basic_map_free(handle_);

    handle_ = other.release();

    return *this;
}

basic_map::~basic_map()
{
    isl_basic_map_free(handle_);
}

isl_basic_map* basic_map::native_handle() const
{
    return handle_;
}

isl_basic_map* basic_map::release() noexcept
{
    isl_basic_map* temp = handle_;

    handle_ = nullptr;

    return temp;
}

void basic_map::add_constraint(constraint c)
{
    handle_ = isl_basic_map_add_constraint(handle_, c.release());
}

basic_map basic_map::universe(space s)
{
    return basic_map(isl_basic_map_universe(s.release()));
}

basic_set wrap(basic_map map)
{
    return basic_set(isl_basic_map_wrap(map.release()));
}

basic_map project_out(basic_map map, isl_dim_type type, unsigned int first, unsigned int n)
{
    return basic_map(isl_basic_map_project_out(map.release(), type, first, n));
}

basic_map apply_range(basic_map lhs, basic_map rhs)
{
    isl_basic_map* temp = isl_basic_map_apply_range(lhs.release(), rhs.release());

    return basic_map(temp);
}

map::map(isl_map* handle_) : handle_{handle_}
{
}

map::map(basic_map other) : handle_{isl_map_from_basic_map(other.release())}
{
}

map::map(const map& other) : handle_{isl_map_copy(other.native_handle())}
{
}

map::map(map&& other) noexcept : handle_{other.release()}
{
}

map::map(const context& ctx_, const std::string& desc_)
: handle_{isl_map_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

map& map::operator=(map other)
{
    isl_map_free(handle_);

    handle_ = other.release();

    return *this;
}

map::~map()
{
    isl_map_free(handle_);
}

isl_map* map::native_handle() const
{
    return handle_;
}

isl_map* map::release() noexcept
{
    isl_map* temp = handle_;

    handle_ = nullptr;

    return temp;
}

space map::get_space() const
{
    return space(isl_map_get_space(handle_));
}

void map::add_constraint(constraint c)
{
    handle_ = isl_map_add_constraint(handle_, c.release());
}

map map::universe(space s)
{
    return map(isl_map_universe(s.release()));
}

map project_out(map m, isl_dim_type type, unsigned int first, unsigned int n)
{
    return map(isl_map_project_out(m.release(), type, first, n));
}

map make_map_from_domain_and_range(set domain, set range)
{
    return map(isl_map_from_domain_and_range(domain.release(), range.release()));
}

union_map::union_map(isl_union_map* handle_) noexcept : handle_{handle_}
{
}

union_map::union_map(const union_map& other) : handle_{isl_union_map_copy(other.native_handle())}
{
}

union_map::union_map(union_map&& other) noexcept : handle_{other.release()}
{
}

union_map::union_map(const context& ctx_, const std::string& desc_)
: handle_{isl_union_map_read_from_str(ctx_.native_handle(), desc_.c_str())}
{
}

union_map::union_map(basic_map other) : handle_{isl_union_map_from_basic_map(other.release())}
{
}

union_map::union_map(map other) : handle_{isl_union_map_from_map(other.release())}
{
}

union_map& union_map::operator=(const union_map& other)
{
    isl_union_map_free(handle_);

    handle_ = isl_union_map_copy(other.native_handle());

    return *this;
}

union_map& union_map::operator=(union_map&& other) noexcept
{
    std::swap(handle_, other.handle_);

    return *this;
}

union_map::~union_map()
{
    isl_union_map_free(handle_);
}

isl_union_map* union_map::native_handle() const
{
    return handle_;
}

isl_union_map* union_map::release() noexcept
{
    isl_union_map* temp = handle_;

    handle_ = nullptr;

    return temp;
}

union_map union_map::empty(space s)
{
    return union_map(isl_union_map_empty(s.release()));
}

union_map apply_domain(union_map lhs, union_map rhs)
{
    isl_union_map* temp = isl_union_map_apply_domain(lhs.release(), rhs.release());

    return union_map(temp);
}

union_map apply_range(union_map lhs, union_map rhs)
{
    isl_union_map* temp = isl_union_map_apply_range(lhs.release(), rhs.release());

    return union_map(temp);
}

union_map intersect(union_map lhs, union_map rhs)
{
    isl_union_map* temp = isl_union_map_intersect(lhs.release(), rhs.release());

    return union_map(temp);
}

union_map intersect_domain(union_map lhs, union_set rhs)
{
    isl_union_map* temp = isl_union_map_intersect_domain(lhs.release(), rhs.release());

    return union_map(temp);
}

union_map union_(union_map lhs, union_map rhs)
{
    isl_union_map* temp = isl_union_map_union(lhs.release(), rhs.release());

    return union_map(temp);
}

union_map add_map(union_map umap, map m)
{
    return union_map(isl_union_map_add_map(umap.release(), m.release()));
}
}
}
}
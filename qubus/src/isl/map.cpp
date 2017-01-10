#include <qbb/qubus/isl/map.hpp>

#include <qbb/qubus/isl/context.hpp>

namespace qbb
{
namespace qubus
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

std::string basic_map::get_tuple_name(isl_dim_type type) const
{
    return std::string(isl_basic_map_get_tuple_name(handle_, type));
}

void basic_map::set_tuple_name(isl_dim_type type, const std::string& name)
{
    handle_ = isl_basic_map_set_tuple_name(handle_, type, name.c_str());
}

void basic_map::set_tuple_id(isl_dim_type type, id val)
{
    handle_ = isl_basic_map_set_tuple_id(handle_, type, val.release());
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

basic_map basic_map::identity(space s)
{
    return basic_map(isl_basic_map_identity(s.release()));
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

basic_map align_params(basic_map map, space model)
{
    return basic_map(isl_basic_map_align_params(map.release(), model.release()));
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

context_ref map::get_ctx() const
{
    return context_ref(isl_map_get_ctx(handle_));
}

bool operator==(const basic_map& lhs, const basic_map& rhs)
{
    return isl_basic_map_is_equal(lhs.native_handle(), rhs.native_handle());
}

bool operator!=(const basic_map& lhs, const basic_map& rhs)
{
    return !(lhs == rhs);
}

set map::domain() const
{
    return set(isl_map_domain(isl_map_copy(handle_)));
}

set map::range() const
{
    return set(isl_map_range(isl_map_copy(handle_)));
}

bool map::is_injective() const
{
    return isl_map_is_injective(handle_);
}

std::string map::get_tuple_name(isl_dim_type type) const
{
    return std::string(isl_map_get_tuple_name(handle_, type));
}

void map::set_tuple_name(isl_dim_type type, const std::string& name)
{
    handle_ = isl_map_set_tuple_name(handle_, type, name.c_str());
}

id map::get_tuple_id(isl_dim_type type) const
{
    return id(isl_map_get_tuple_id(handle_, type));
}

void map::set_tuple_id(isl_dim_type type, id val)
{
    handle_ = isl_map_set_tuple_id(handle_, type, val.release());
}

id map::get_dim_id(isl_dim_type type, unsigned pos) const
{
    return id(isl_map_get_dim_id(handle_, type, pos));
}

std::string map::get_dim_name(isl_dim_type type, unsigned pos) const
{
    return std::string(isl_map_get_dim_name(handle_, type, pos));
}

void map::set_dim_id(isl_dim_type type, unsigned pos, id value)
{
    handle_ = isl_map_set_dim_id(handle_, type, pos, value.release());
}

int map::dim(isl_dim_type type) const
{
    return isl_map_dim(handle_, type);
}

void map::add_constraint(constraint c)
{
    handle_ = isl_map_add_constraint(handle_, c.release());
}

map map::universe(space s)
{
    return map(isl_map_universe(s.release()));
}

map map::empty(space s)
{
    return map(isl_map_empty(s.release()));
}

map map::identity(space s)
{
    return map(isl_map_identity(s.release()));
}

map map::from_multi_affine_expr(multi_affine_expr expr)
{
    return map(isl_map_from_multi_aff(expr.release()));
}

map map::from_affine_expr(affine_expr expr)
{
    return map(isl_map_from_aff(expr.release()));
}

bool operator==(const map& lhs, const map& rhs)
{
    return isl_map_is_equal(lhs.native_handle(), rhs.native_handle());
}

bool operator!=(const map& lhs, const map& rhs)
{
    return !(lhs == rhs);
}

map project_out(map m, isl_dim_type type, unsigned int first, unsigned int n)
{
    return map(isl_map_project_out(m.release(), type, first, n));
}

map make_map_from_domain_and_range(set domain, set range)
{
    return map(isl_map_from_domain_and_range(domain.release(), range.release()));
}

map apply_domain(map lhs, map rhs)
{
    isl_map* temp = isl_map_apply_domain(lhs.release(), rhs.release());

    return map(temp);
}

map apply_range(map lhs, map rhs)
{
    isl_map* temp = isl_map_apply_range(lhs.release(), rhs.release());

    return map(temp);
}

set apply(map m, set values)
{
    return set(isl_set_apply(values.release(), m.release()));
}

map union_(map lhs, map rhs)
{
    return map(isl_map_union(lhs.release(), rhs.release()));
}

map intersect(map lhs, map rhs)
{
    return map(isl_map_intersect(lhs.release(), rhs.release()));
}

map intersect_domain(map lhs, set rhs)
{
    return map(isl_map_intersect_domain(lhs.release(), rhs.release()));
}

map intersect_range(map lhs, set rhs)
{
    return map(isl_map_intersect_range(lhs.release(), rhs.release()));
}

map add_dims(map m, isl_dim_type type, int num)
{
    return map(isl_map_add_dims(m.release(), type, num));
}

map fix_dimension(map m, isl_dim_type type, int pos, int value)
{
    return map(isl_map_fix_si(m.release(), type, pos, value));
}

map flat_product(map lhs, map rhs)
{
    return map(isl_map_flat_product(lhs.release(), rhs.release()));
}

map flat_range_product(map lhs, map rhs)
{
    return map(isl_map_flat_range_product(lhs.release(), rhs.release()));
}

map align_params(map m, space s)
{
    return map(isl_map_align_params(m.release(), s.release()));
}

map reverse(map m)
{
    return map(isl_map_reverse(m.release()));
}

map coalesce(map m)
{
    return map(isl_map_coalesce(m.release()));
}

map detect_equalities(map m)
{
    return map(isl_map_detect_equalities(m.release()));
}

map remove_redundancies(map m)
{
    return map(isl_map_remove_redundancies(m.release()));
}

map simplify(map m)
{
    return remove_redundancies(detect_equalities(coalesce(std::move(m))));
}

set wrap(map m)
{
    return set(isl_map_wrap(m.release()));
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

space union_map::get_space() const
{
    return space(isl_union_map_get_space(handle_));
}

context_ref union_map::get_ctx() const
{
    return context_ref(isl_union_map_get_ctx(handle_));
}

namespace
{
extern "C" isl_stat add_map(isl_map* m, void* user) noexcept;
}

std::vector<map> union_map::get_maps() const
{
    std::vector<map> result;

    isl_union_map_foreach_map(handle_, add_map, &result);

    return result;
}

union_set union_map::domain() const
{
    return union_set(isl_union_map_domain(isl_union_map_copy(handle_)));
}

union_set union_map::range() const
{
    return union_set(isl_union_map_range(isl_union_map_copy(handle_)));
}

bool union_map::is_injective() const
{
    return isl_union_map_is_injective(handle_);
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

bool operator==(const union_map& lhs, const union_map& rhs)
{
    return isl_union_map_is_equal(lhs.native_handle(), rhs.native_handle());
}

bool operator!=(const union_map& lhs, const union_map& rhs)
{
    return !(lhs == rhs);
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

union_map substract(union_map lhs, union_map rhs)
{
    return union_map(isl_union_map_subtract(lhs.release(), rhs.release()));
}

union_map add_map(union_map umap, map m)
{
    return union_map(isl_union_map_add_map(umap.release(), m.release()));
}

union_map flat_range_product(union_map lhs, union_map rhs)
{
    return union_map(isl_union_map_flat_range_product(lhs.release(), rhs.release()));
}

union_map align_params(union_map m, space s)
{
    return union_map(isl_union_map_align_params(m.release(), s.release()));
}

union_map project_out(union_map m, isl_dim_type type, unsigned int first, unsigned int n)
{
    return union_map(isl_union_map_project_out(m.release(), type, first, n));
}

union_map coalesce(union_map m)
{
    return union_map(isl_union_map_coalesce(m.release()));
}

union_map detect_equalities(union_map m)
{
    return union_map(isl_union_map_detect_equalities(m.release()));
}

union_map remove_redundancies(union_map m)
{
    return union_map(isl_union_map_remove_redundancies(m.release()));
}

union_map simplify(union_map m)
{
    return remove_redundancies(detect_equalities(coalesce(std::move(m))));
}

namespace
{
extern "C" {

isl_stat add_map(isl_map* m, void* user) noexcept
{
    auto& maps = *static_cast<std::vector<map>*>(user);

    maps.emplace_back(m);

    return isl_stat_ok;
}
}
}

}
}
}
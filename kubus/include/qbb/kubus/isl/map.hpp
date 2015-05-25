#ifndef QBB_KUBUS_ISL_MAP_HPP
#define QBB_KUBUS_ISL_MAP_HPP

#include <qbb/kubus/isl/space.hpp>
#include <qbb/kubus/isl/constraint.hpp>
#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/context.hpp>

#include <isl/map.h>
#include <isl/union_map.h>

#include <string>
#include <vector>

namespace qbb
{
namespace kubus
{
namespace isl
{

class context;
    
class basic_map
{
public:
    explicit basic_map(isl_basic_map* handle_);

    basic_map(const basic_map& other);

    basic_map(basic_map&& other) noexcept;

    basic_map(const context& ctx_, const std::string& desc_);

    basic_map& operator=(basic_map other);

    ~basic_map();

    isl_basic_map* native_handle() const;

    isl_basic_map* release() noexcept;

    void set_tuple_name(isl_dim_type type, const std::string& name);

    void add_constraint(constraint c);

    static basic_map universe(space s);
    static basic_map identity(space s);
private:
    isl_basic_map* handle_;
};

basic_set wrap(basic_map map);

basic_map project_out(basic_map map, isl_dim_type type, unsigned int first, unsigned int n);

basic_map apply_range(basic_map lhs, basic_map rhs);

basic_map align_params(basic_map map, space model);

class map
{
public:
    explicit map(isl_map* handle_);

    map(basic_map other);

    map(const map& other);

    map(map&& other) noexcept;

    map(const context& ctx_, const std::string& desc_);

    map& operator=(map other);

    ~map();

    isl_map* native_handle() const;

    isl_map* release() noexcept;

    space get_space() const;
    context_ref get_ctx() const;

    std::string get_tuple_name(isl_dim_type type) const;

    int dim(isl_dim_type type) const;

    void add_constraint(constraint c);
    
    static map universe(space s);
    static map empty(space s);
private:
    isl_map* handle_;
};

map project_out(map m, isl_dim_type type, unsigned int first, unsigned int n);

map make_map_from_domain_and_range(set domain, set range);

map apply_domain(map lhs, map rhs);

map apply_range(map lhs, map rhs);

map union_(map lhs, map rhs);

map add_dims(map m, isl_dim_type type, int num);

map fix_dimension(map m, isl_dim_type type, int pos, int value);

inline set apply(set s, map m)
{
    return set(isl_set_apply(s.release(), m.release()));
}

class union_map
{
public:
    explicit union_map(isl_union_map* handle_) noexcept;

    union_map(const union_map& other);

    union_map(union_map&& other) noexcept;

    union_map(const context& ctx_, const std::string& desc_);

    union_map(basic_map other);

    union_map(map other);

    union_map& operator=(const union_map& other);

    union_map& operator=(union_map&& other) noexcept;

    ~union_map();

    space get_space() const;
    context_ref get_ctx() const;
    
    std::vector<map> get_maps() const;
    
    union_set domain() const;
    union_set range() const;
    
    isl_union_map* native_handle() const;

    isl_union_map* release() noexcept;
    
    static union_map empty(space s);

private:
    isl_union_map* handle_;
};

union_map apply_domain(union_map lhs, union_map rhs);

union_map apply_range(union_map lhs, union_map rhs);

union_map intersect(union_map lhs, union_map rhs);

union_map intersect_domain(union_map lhs, union_set rhs);

union_map union_(union_map lhs, union_map rhs);

union_map add_map(union_map umap, map m);

union_map flat_range_product(union_map lhs, union_map rhs);

union_map align_params(union_map m, space s);

union_map project_out(union_map m, isl_dim_type type, unsigned int first, unsigned int n);

inline union_set apply(union_set s, union_map m)
{
    return union_set(isl_union_set_apply(s.release(), m.release()));
}

inline union_map reverse(union_map m)
{
    return union_map(isl_union_map_reverse(m.release()));
}

}
}
}

#endif
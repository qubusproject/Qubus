#ifndef QBB_KUBUS_ISL_MAP_HPP
#define QBB_KUBUS_ISL_MAP_HPP

#include <qbb/kubus/isl/space.hpp>
#include <qbb/kubus/isl/constraint.hpp>
#include <qbb/kubus/isl/set.hpp>

#include <isl/map.h>

#include <string>

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

    void add_constraint(constraint c);

    static basic_map universe(space s);

private:
    isl_basic_map* handle_;
};

basic_set wrap(basic_map map);

basic_map project_out(basic_map map, isl_dim_type type, unsigned int first, unsigned int n);

basic_map apply_range(basic_map lhs, basic_map rhs);

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

    void add_constraint(constraint c);
    
    static map universe(space s);

private:
    isl_map* handle_;
};

map project_out(map m, isl_dim_type type, unsigned int first, unsigned int n);

map make_map_from_domain_and_range(set domain, set range);

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
}
}
}

#endif
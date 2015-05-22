#ifndef QBB_KUBUS_ISL_SET_HPP
#define QBB_KUBUS_ISL_SET_HPP

#include <qbb/kubus/isl/space.hpp>
#include <qbb/kubus/isl/constraint.hpp>

#include <isl/set.h>
#include <isl/union_set.h>

#include <string>

namespace qbb
{
namespace kubus
{
namespace isl
{

class context;

class basic_set
{
public:
    explicit basic_set(isl_basic_set* handle_);

    basic_set(const context& ctx_, const std::string& desc_);

    basic_set(const basic_set& other);

    ~basic_set();

    isl_basic_set* native_handle() const;

    isl_basic_set* release() noexcept;
    
    void add_constraint(constraint c);
    
    space get_space() const;

    void set_tuple_name(const std::string& name);

    static basic_set universe(space s);
private:
    isl_basic_set* handle_;
};

bool is_subset(const basic_set& lhs, const basic_set& rhs);

bool is_empty(const basic_set& s);

class set
{
public:
    explicit set(isl_set* handle_);

    set(basic_set other);

    set(const set& other);

    set(const context& ctx_, const std::string& desc_);

    ~set();
    
    set& operator=(const set& other);

    isl_set* native_handle() const;

    isl_set* release() noexcept;

    void add_constraint(constraint c);

    space get_space() const;
    
    void set_tuple_name(const std::string& name);
    
    static set universe(space s);
    static set empty(space s);

private:
    isl_set* handle_;
};

set intersect(set lhs, set rhs);

set intersect_params(set lhs, set rhs);

set substract(set lhs, set rhs);

bool is_subset(const set& lhs, const set& rhs);

bool is_strict_subset(const set& lhs, const set& rhs);

bool is_empty(const set& s);

set flat_product(set lhs, set rhs);

set align_params(set s, space model);

set project_out(set s, isl_dim_type type, unsigned int first, unsigned int n);

class union_set
{
public:
    explicit union_set(isl_union_set* handle_);    
    
    union_set(basic_set other);

    union_set(set other);

    union_set(const union_set& other);

    union_set(const context& ctx_, const std::string& desc_);

    ~union_set();
    
    union_set& operator=(const union_set& other);

    isl_union_set* native_handle() const;

    isl_union_set* release() noexcept;

    static union_set empty(space s);
private:
    isl_union_set* handle_;
};

union_set union_(union_set lhs, union_set rhs);
union_set intersect(union_set lhs, union_set rhs);

bool is_subset(const union_set& lhs, const union_set& rhs);

bool is_strict_subset(const union_set& lhs, const union_set& rhs);

bool is_empty(const union_set& s);

union_set add_set(union_set uset, set s);

}
}
}

#endif
#ifndef QUBUS_ISL_SET_HPP
#define QUBUS_ISL_SET_HPP

#include <qubus/isl/space.hpp>
#include <qubus/isl/constraint.hpp>

#include <isl/set.h>
#include <isl/union_set.h>

#include <boost/utility/string_ref.hpp>

#include <string>
#include <vector>

namespace qubus
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
    basic_set(basic_set&& other) noexcept;

    ~basic_set();

    basic_set& operator=(const basic_set& other);
    basic_set& operator=(basic_set&& other) noexcept;

    isl_basic_set* native_handle() const;

    isl_basic_set* release() noexcept;
    
    void add_constraint(constraint c);
    
    space get_space() const;

    void set_tuple_name(const std::string& name);

    static basic_set universe(space s);
private:
    isl_basic_set* handle_;
};

bool operator==(const basic_set& lhs, const basic_set& rhs);
bool operator!=(const basic_set& lhs, const basic_set& rhs);

bool is_subset(const basic_set& lhs, const basic_set& rhs);

bool is_empty(const basic_set& s);

basic_set flat_product(basic_set lhs, basic_set rhs);

basic_set align_params(basic_set s, space model);

basic_set add_dims(basic_set s, isl_dim_type type, unsigned int n);

basic_set project_out(basic_set s, isl_dim_type type, unsigned int first, unsigned int n);

class set
{
public:
    explicit set(isl_set* handle_);

    set(basic_set other);

    set(const set& other);
    set(set&& other) noexcept;

    set(const context& ctx_, const std::string& desc_);

    ~set();
    
    set& operator=(const set& other);
    set& operator=(set&& other) noexcept;

    isl_set* native_handle() const;

    isl_set* release() noexcept;

    void add_constraint(constraint c);

    space get_space() const;

    int dim(isl_dim_type type) const;

    set params() const
    {
        return set(isl_set_params(handle_));
    }
    
    void set_tuple_name(const std::string& name);
    std::string get_tuple_name() const;

    void set_dim_name(isl_dim_type type, int pos, const std::string& name);
    boost::string_ref get_dim_name(isl_dim_type type, int pos) const;

    bool bounded() const;

    static set universe(space s);
    static set empty(space s);

    static set from_params(set s)
    {
        return set(isl_set_from_params(s.native_handle()));
    }
private:
    isl_set* handle_;
};

set union_(set lhs, set rhs);

set intersect(set lhs, set rhs);

set intersect_params(set lhs, set rhs);

set substract(set lhs, set rhs);

set complement(set arg);

bool operator==(const set& lhs, const set& rhs);
bool operator!=(const set& lhs, const set& rhs);

bool is_subset(const set& lhs, const set& rhs);

bool is_strict_subset(const set& lhs, const set& rhs);

bool is_empty(const set& s);

set get_params(set s);

set flat_product(set lhs, set rhs);

set align_params(set s, space model);

set project_out(set s, isl_dim_type type, unsigned int first, unsigned int n);

set lexmin(set s);
set lexmax(set s);

set add_dims(set s, isl_dim_type type, unsigned int n);

set coalesce(set s);
set detect_equalities(set s);
set remove_redundancies(set s);
set simplify(set s);

class union_set
{
public:
    explicit union_set(isl_union_set* handle_);    
    
    union_set(basic_set other);

    union_set(set other);

    union_set(const union_set& other);
    union_set(union_set&& other) noexcept;

    union_set(const context& ctx_, const std::string& desc_);

    ~union_set();
    
    union_set& operator=(const union_set& other);
    union_set& operator=(union_set&& other) noexcept;

    std::vector<set> get_sets() const;

    isl_union_set* native_handle() const;

    isl_union_set* release() noexcept;

    static union_set empty(space s);
private:
    isl_union_set* handle_;
};

union_set union_(union_set lhs, union_set rhs);
union_set intersect(union_set lhs, union_set rhs);

bool operator==(const union_set& lhs, const union_set& rhs);
bool operator!=(const union_set& lhs, const union_set& rhs);

bool is_subset(const union_set& lhs, const union_set& rhs);

bool is_strict_subset(const union_set& lhs, const union_set& rhs);

bool is_empty(const union_set& s);

union_set add_set(union_set uset, set s);
set extract_set(const union_set& uset, space s);

union_set coalesce(union_set s);
union_set detect_equalities(union_set s);
union_set remove_redundancies(union_set s);
union_set simplify(union_set s);

}
}

#endif
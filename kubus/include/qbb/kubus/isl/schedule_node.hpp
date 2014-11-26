#ifndef QBB_KUBUS_ISL_SCHEDULE_NODE_HPP
#define QBB_KUBUS_ISL_SCHEDULE_NODE_HPP

#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/multi_union_pw_affine_expr.hpp>

#include <isl/schedule_node.h>

#include <vector>

namespace qbb
{
namespace kubus
{
namespace isl
{

class schedule_node
{
public:
    explicit schedule_node(isl_schedule_node* handle_);
    
    schedule_node(const schedule_node& other);
    
    ~schedule_node();
    
    schedule_node& operator=(const schedule_node& other);
    
    isl_schedule_node_type get_type() const;
    
    schedule_node operator[](int idx);
    int n_children() const;
    bool has_children() const;
    
    schedule_node parent() const;
    
    int band_n_member() const;
    bool band_is_permutable() const;
    void band_member_set_ast_loop_type(int pos, isl_ast_loop_type type);
    void band_set_ast_build_options(union_set options);
    
    isl_schedule_node* native_handle() const;
    isl_schedule_node* release() noexcept;
private:
    isl_schedule_node* handle_;
};

schedule_node insert_partial_schedule(schedule_node parent, multi_union_pw_affine_expr schedule);
schedule_node insert_context(schedule_node parent, set context);
schedule_node insert_filter(schedule_node parent, union_set filter);
schedule_node insert_set(schedule_node parent, std::vector<union_set> filters);
schedule_node insert_sequence(schedule_node parent, std::vector<union_set> filters);

schedule_node tile_band(schedule_node node, std::vector<long int> sizes);

}
}
}

#endif
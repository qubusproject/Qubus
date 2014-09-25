#ifndef QBB_KUBUS_ISL_AST_BUILDER_HPP
#define QBB_KUBUS_ISL_AST_BUILDER_HPP

#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/map.hpp>
#include <qbb/kubus/isl/ast.hpp>

#include <isl/ast_build.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class ast_builder
{
public:
    ast_builder(const ast_builder& other) = delete;

    ast_builder(set params);

    ~ast_builder();

    ast_builder& operator=(const ast_builder& other) = delete;

    ast_node build_ast_from_schedule(union_map schedule);

    void set_options(union_map options);

    isl_ast_build* native_handle() const;

private:
    isl_ast_build* handle_;
};

}
}
}

#endif
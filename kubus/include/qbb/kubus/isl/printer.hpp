#ifndef QBB_KUBUS_ISL_PRINTER_HPP
#define QBB_KUBUS_ISL_PRINTER_HPP

#include <isl/printer.h>
#include <isl/ast.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class ast_node;
class union_map;
class basic_set;
class set;
class context;
    
class printer
{
public:
    printer(const printer& other) = delete;

    printer(const context& ctx_);

    ~printer();

    printer& operator=(const printer& other) = delete;

    void print(const basic_set& set);
    
    void print(const set& s);

    void print(const union_map& map);

    void print(const ast_node& ast);
    
    void print_macro(isl_ast_op_type op_type);

    isl_printer* native_handle() const;

private:
    isl_printer* handle_;
};

}
}
}

#endif
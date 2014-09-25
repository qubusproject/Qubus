#include <qbb/kubus/isl/printer.hpp>

#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/map.hpp>
#include <qbb/kubus/isl/ast.hpp>
#include <qbb/kubus/isl/context.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

printer::printer(const context& ctx_) : handle_{isl_printer_to_str(ctx_.native_handle())}
{
}

printer::~printer()
{
    isl_printer_free(handle_);
}

void printer::print(const basic_set& set)
{
    handle_ = isl_printer_set_output_format(handle_, ISL_FORMAT_ISL);

    handle_ = isl_printer_print_basic_set(handle_, set.native_handle());

    char* buffer = isl_printer_get_str(handle_);

    std::string result = buffer;

    handle_ = isl_printer_flush(handle_);

    free(buffer);

    std::cout << result;
}

void printer::print(const set& s)
{
    handle_ = isl_printer_set_output_format(handle_, ISL_FORMAT_ISL);

    handle_ = isl_printer_print_set(handle_, s.native_handle());

    char* buffer = isl_printer_get_str(handle_);

    std::string result = buffer;

    handle_ = isl_printer_flush(handle_);

    free(buffer);

    std::cout << result;
}

void printer::print(const union_map& map)
{
    handle_ = isl_printer_set_output_format(handle_, ISL_FORMAT_ISL);

    handle_ = isl_printer_print_union_map(handle_, map.native_handle());

    char* buffer = isl_printer_get_str(handle_);

    std::string result = buffer;

    handle_ = isl_printer_flush(handle_);

    free(buffer);

    std::cout << result;
}

void printer::print(const ast_node& ast)
{
    handle_ = isl_printer_set_output_format(handle_, ISL_FORMAT_C);

    handle_ = isl_ast_node_print_macros(ast.native_handle(), handle_);

    handle_ = isl_printer_print_ast_node(handle_, ast.native_handle());

    char* buffer = isl_printer_get_str(handle_);

    std::string result = buffer;

    handle_ = isl_printer_flush(handle_);

    free(buffer);

    std::cout << result;
}

isl_printer* printer::native_handle() const
{
    return handle_;
}
}
}
}
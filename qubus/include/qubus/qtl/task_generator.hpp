#ifndef QUBUS_QTL_TASK_GENERATOR_HPP
#define QUBUS_QTL_TASK_GENERATOR_HPP

#include <qubus/IR/function_declaration.hpp>

#include <memory>

namespace qubus
{

class expression;

namespace qtl
{

function_declaration wrap_code_in_task(std::unique_ptr<expression> expr);

}
}

#endif

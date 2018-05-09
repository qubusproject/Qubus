#ifndef QUBUS_QTL_TASK_GENERATOR_HPP
#define QUBUS_QTL_TASK_GENERATOR_HPP

#include <qubus/IR/module.hpp>

#include <memory>

namespace qubus
{

class expression;

namespace qtl
{

std::unique_ptr<module> wrap_code_in_task(std::unique_ptr<expression> expr);

}
}

#endif

#ifndef QUBUS_QTL_OBJECT_EXTRACTION_PASS_HPP
#define QUBUS_QTL_OBJECT_EXTRACTION_PASS_HPP

#include <qubus/object.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <memory>
#include <tuple>
#include <vector>

namespace qubus
{
namespace qtl
{
std::unique_ptr<expression>
extract_objects(const expression& expr,
                std::vector<std::tuple<variable_declaration, object>>& parameter_map);
}
}

#endif

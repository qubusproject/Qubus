#ifndef QBB_QUBUS_COMPILER_HPP
#define QBB_QUBUS_COMPILER_HPP

#include <qbb/kubus/IR/function_declaration.hpp>
#include <qbb/kubus/plan.hpp>

namespace qbb
{
namespace qubus
{

class compiler
{
public:
    compiler() = default;
    compiler(const compiler&) = delete;
    
    virtual ~compiler() = default;
    
    compiler& operator=(const compiler&) = delete;
    
    virtual plan compile_plan(const function_declaration& plan_decl) = 0;
};

}
}

#endif
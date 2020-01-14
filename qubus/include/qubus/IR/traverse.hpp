#ifndef QQUBUS_IR_TRAVERSE_HPP
#define QUBUS_IR_TRAVERSE_HPP

namespace qubus
{

class visitor;
class expression;

void traverse(const expression& expr, visitor& v);

}

#endif

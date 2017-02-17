#ifndef QUBUS_PATTERN_IR_HPP
#define QUBUS_PATTERN_IR_HPP

#include <qubus/pattern/binary_operator.hpp>
#include <qubus/pattern/unary_operator.hpp>
#include <qubus/pattern/subscription.hpp>
#include <qubus/pattern/type_conversion.hpp>
#include <qubus/pattern/compound.hpp>
#include <qubus/pattern/intrinsic_function.hpp>
#include <qubus/pattern/literal.hpp>
#include <qubus/pattern/for.hpp>
#include <qubus/pattern/variable_ref.hpp>
#include <qubus/pattern/macro.hpp>
#include <qubus/pattern/spawn.hpp>
#include <qubus/pattern/local_variable_def.hpp>
#include <qubus/pattern/variable_scope.hpp>
#include <qubus/pattern/construct.hpp>
#include <qubus/pattern/if.hpp>
#include <qubus/pattern/member_access.hpp>
#include <qubus/pattern/foreign_call.hpp>
#include <qubus/pattern/access.hpp>
#include <qubus/pattern/array_slice.hpp>

#include <qubus/pattern/type.hpp>

#include <qubus/pattern/contains.hpp>

#include <qubus/pattern/substitute.hpp>
#include <qubus/pattern/search.hpp>
#include <qubus/pattern/for_each.hpp>
#include <qubus/pattern/fold.hpp>

#endif
#ifndef QUBUS_IR_COMPILE_TIME_EVALUATOR_HPP
#define QUBUS_IR_COMPILE_TIME_EVALUATOR_HPP

#include <qubus/IR/type.hpp>
#include <qubus/IR/expression.hpp>

#include <variant>
#include <cstdint>

namespace qubus
{

class compile_time_value
{
public:
    explicit compile_time_value(std::int64_t value);
    explicit compile_time_value(std::int32_t value);
    explicit compile_time_value(std::int16_t value);
    explicit compile_time_value(std::int8_t value);
    explicit compile_time_value(float value);
    explicit compile_time_value(double value);
    explicit compile_time_value(type value);


private:
    std::variant<
      std::int64_t,
      std::int32_t,
      std::int16_t,
      std::int8_t,
      float,
      double,
      type> m_value;
};

class compile_time_evaluator
{
public:
    virtual ~compile_time_evaluator() noexcept = default;

    virtual compile_time_value evaluate(const expression& expr) const = 0;
};

}

#endif

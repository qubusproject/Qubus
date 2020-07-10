#ifndef QUBUS_PATTERN_SYMBOL_HPP
#define QUBUS_PATTERN_SYMBOL_HPP

#include <qubus/IR/symbol_expr.hpp>

#include <utility>

namespace qubus::pattern
{

template <typename IDPattern>
class symbol_pattern
{
public:
    explicit symbol_pattern(IDPattern id)
        : m_id(std::move(id))
    {
    }

    bool match(const expression& value,
               const variable<const symbol_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<symbol_expr>())
        {
            if (m_id.match(concret_value->id()))
            {
                if (var != nullptr)
                {
                    var->set(concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        m_id.reset();
    }

private:
    IDPattern m_id;
};

template <typename IDPattern>
auto symbol(IDPattern id)
{
    return symbol_pattern<IDPattern>(std::move(id));
}

}

#endif

#ifndef QUBUS_PATTERN_LEXICAL_SCOPE_HPP
#define QUBUS_PATTERN_LEXICAL_SCOPE_HPP

#include <qubus/pattern/any.hpp>

#include <utility>

namespace qubus::pattern
{

template <typename GuardPattern>
class lexical_scope_pattern
{
public:
    explicit lexical_scope_pattern(GuardPattern guard_pattern)
    : m_guard_pattern(std::move(guard_pattern))
    {
    }

    bool match(const expression& value,
               const variable<const expression&>* var = nullptr) const
    {
        if (value.is_lexical_scope() && m_guard_pattern.match(value))
        {
            if (var != nullptr)
            {
                var->set(value);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
        m_guard_pattern.reset();
    }

private:
    GuardPattern m_guard_pattern;
};

template <typename GuardPattern>
auto lexical_scope(GuardPattern guard_pattern)
{
    return lexical_scope_pattern<GuardPattern>(std::move(guard_pattern));
}

auto lexical_scope()
{
    return lexical_scope(_);
}

}
}

#endif

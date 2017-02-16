#ifndef QBB_QUBUS_PATTERN_ANY_OF_HPP
#define QBB_QUBUS_PATTERN_ANY_OF_HPP

#include <qbb/qubus/pattern/variable.hpp>

#include <utility>
#include <cstddef>

namespace qubus
{
namespace pattern
{

template <typename Pattern>
class any_of_pattern
{
public:
    explicit any_of_pattern(Pattern pattern_) : pattern_(std::move(pattern_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& seq, const variable<std::size_t>* var = nullptr) const
    {
        std::size_t current_pos = 0;

        for (const auto& value : seq)
        {
            if (pattern_.match(value))
            {
                if (var)
                {
                    var->set(current_pos);
                }

                return true;
            }
            
            ++current_pos;
        }

        return false;
    }

    void reset() const
    {
        pattern_.reset();
    }
private:
    Pattern pattern_;
};

template <typename Pattern>
any_of_pattern<Pattern> any_of(Pattern pattern)
{
    return any_of_pattern<Pattern>(pattern);
}
}
}

#endif
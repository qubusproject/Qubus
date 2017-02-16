#ifndef QBB_QUBUS_PATTERN_CONTAINS_HPP
#define QBB_QUBUS_PATTERN_CONTAINS_HPP

#include <qbb/qubus/IR/expression.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/search.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{

template <typename Pattern>
class contains_pattern
{
public:
    contains_pattern(Pattern pattern_) : pattern_(std::move(pattern_))
    {
    }

    bool match(const expression& value, const variable<std::reference_wrapper<const expression>>* var = nullptr) const
    {
        //TODO: Refactor this code after we have added a version of search with result type void.
        auto m = make_matcher<expression, bool>().case_(protect(pattern_), []
                                                              {
                                                                  return true;
                                                              });

        auto result = search(value, m);

        if (result)
        {
            if (var)
            {
                var->set(value);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    void reset() const
    {
        pattern_.reset();
    }

private:
    Pattern pattern_;
};

template <typename Pattern>
contains_pattern<Pattern> contains(Pattern pattern)
{
    return contains_pattern<Pattern>(pattern);
}
}
}

#endif

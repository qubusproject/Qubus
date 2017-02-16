#ifndef QBB_PATTERN_ALL_OF_HPP
#define QBB_PATTERN_ALL_OF_HPP

#include <qubus/pattern/variable.hpp>
#include <qubus/pattern/pattern_traits.hpp>

#include <qubus/util/function_traits.hpp>

#include <vector>
#include <utility>

namespace qubus
{
namespace pattern
{

template <typename Pattern>
class all_of_pattern
{
public:
    explicit all_of_pattern(Pattern pattern_) : pattern_(std::move(pattern_))
    {
    }

    template <typename Sequence>
    bool match(const Sequence& sequence, const variable<std::vector<typename pattern_traits<Pattern>::template value_type<Sequence>>>* var = nullptr) const
    {
        using result_type = typename pattern_traits<Pattern>::template value_type<Sequence>;

        std::vector<result_type> result;

        for (const auto& value : sequence)
        {
            variable<result_type> var;

            if (!pattern_.match(value, &var))
                return false;

            result.push_back(var.get());

            pattern_.reset();
        }

        if (var)
        {
            var->set(result);
        }

        return true;
    }

    void reset() const
    {
        pattern_.reset();
    }

private:
    Pattern pattern_;
};

template <typename Pattern>
all_of_pattern<Pattern> all_of(Pattern pattern)
{
    return all_of_pattern<Pattern>(std::move(pattern));
}

}
}

#endif

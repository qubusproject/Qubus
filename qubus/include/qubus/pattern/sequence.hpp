#ifndef QUBUS_PATTERN_SEQUENCE_HPP
#define QUBUS_PATTERN_SEQUENCE_HPP

#include <qubus/pattern/variable.hpp>

#include <qubus/util/integers.hpp>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/algorithm/iteration/fold.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>

#include <utility>

namespace qubus
{
namespace pattern
{
template <typename... Values>
class sequence_pattern
{
public:
    explicit sequence_pattern(Values... values_) : values_(std::move(values_)...)
    {
    }

    template <typename Sequence>
    bool match(const Sequence& sequence, const variable<Sequence>* var = nullptr) const
    {
        if (size(values_) == util::to_uindex(sequence.size()))
        {
            if (boost::fusion::fold(values_, true, [&, i = 0 ](bool acc, const auto& value) mutable
                                    {
                    bool result = acc && value.match(sequence[i]);
                    ++i;
                    return result;
                }))
            {
                if (var)
                {
                    var->set(sequence);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        boost::fusion::for_each(values_, [](const auto& value) { value.reset(); });
    }
private:
    boost::fusion::vector<Values...> values_;
};

template <typename... Values>
sequence_pattern<Values...> sequence(Values... values)
{
    return sequence_pattern<Values...>(values...);
}

}
}

#endif
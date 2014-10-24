#ifndef QBB_KUBUS_PATTERN_SEQUENCE_HPP
#define QBB_KUBUS_PATTERN_SEQUENCE_HPP

#include <qbb/kubus/pattern/variable.hpp>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/algorithm/iteration/fold.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename... Values>
class sequence_pattern
{
public:
    sequence_pattern(Values... values_) : values_(std::move(values_)...)
    {
    }

    template <typename Sequence>
    bool match(const Sequence& sequence, const variable<Sequence>* var = nullptr) const
    {
        if (size(values_) == sequence.size())
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
}

#endif
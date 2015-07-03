#ifndef QBB_KUBUS_PATTERN_SUM_HPP
#define QBB_KUBUS_PATTERN_SUM_HPP

#include <qbb/kubus/IR/sum_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Body, typename Indices>
class sum_pattern
{
public:
    sum_pattern(Body body_, Indices indices_)
    : body_(std::move(body_)), indices_(std::move(indices_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<sum_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<sum_expr>())
        {
            if (body_.match(concret_value->body()))
            {
                if (indices_.match(concret_value->contraction_indices()))
                {
                    if (var)
                    {
                        var->set(*concret_value);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    void reset() const
    {
        body_.reset();
        indices_.reset();
    }

private:
    Body body_;
    Indices indices_;
};

template <typename Body, typename Indices, typename Alias>
class sum_with_alias_pattern
{
public:
    sum_with_alias_pattern(Body body_, Indices indices_, Alias alias_)
    : body_(std::move(body_)), indices_(std::move(indices_)), alias_(std::move(alias_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<sum_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<sum_expr>())
        {
            if (body_.match(concret_value->body()))
            {
                if (concret_value->alias())
                {
                    if (alias_.match(*concret_value->alias()))
                    {
                        if (indices_.match(concret_value->contraction_indices()))
                        {
                            if (var)
                            {
                                var->set(*concret_value);
                            }

                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    void reset() const
    {
        body_.reset();
        indices_.reset();
        alias_.reset();
    }

private:
    Body body_;
    Indices indices_;
    Alias alias_;
};

template <typename Body, typename Indices>
sum_pattern<Body, Indices> sum_multi(Body body, Indices indices)
{
    return sum_pattern<Body, Indices>(body, indices);
}

template <typename Body, typename Indices, typename Alias>
sum_with_alias_pattern<Body, Indices, Alias> sum_multi(Body body, Indices indices, Alias alias)
{
    return sum_with_alias_pattern<Body, Indices, Alias>(body, indices, alias);
}

template <typename Body, typename Index>
auto sum(Body body, Index index)
{
    return sum_multi(body, sequence(index));
}
}
}
}

#endif
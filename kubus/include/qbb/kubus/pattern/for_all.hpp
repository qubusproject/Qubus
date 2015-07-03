#ifndef QBB_KUBUS_PATTERN_FOR_ALL_HPP
#define QBB_KUBUS_PATTERN_FOR_ALL_HPP

#include <qbb/kubus/IR/for_all_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Indices, typename Body>
class for_all_pattern
{
public:
    for_all_pattern(Indices indices_, Body body_)
    : indices_(std::move(indices_)), body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<for_all_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<for_all_expr>())
        {
            if (indices_.match(concret_value->loop_indices()))
            {
                if (body_.match(concret_value->body()))
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
        indices_.reset();
        body_.reset();
    }

private:
    Indices indices_;
    Body body_;
};

template <typename Indices, typename Alias, typename Body>
class for_all_with_alias_pattern
{
public:
    for_all_with_alias_pattern(Indices indices_, Alias alias_, Body body_)
    : indices_(std::move(indices_)), alias_(std::move(alias_)), body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<for_all_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<for_all_expr>())
        {
            if (indices_.match(concret_value->loop_indices()))
            {
                if (concret_value->alias())
                {
                    if (alias_.match(*concret_value->alias()))
                    {
                        if (body_.match(concret_value->body()))
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
        indices_.reset();
        alias_.reset();
        body_.reset();
    }

private:
    Indices indices_;
    Alias alias_;
    Body body_;
};

template <typename Indices, typename Body>
for_all_pattern<Indices, Body> for_all_multi(Indices indices, Body body)
{
    return for_all_pattern<Indices, Body>(indices, body);
}

template <typename Indices, typename Alias, typename Body>
for_all_with_alias_pattern<Indices, Alias, Body> for_all_multi(Indices indices, Alias alias, Body body)
{
    return for_all_with_alias_pattern<Indices, Alias, Body>(indices, alias, body);
}

template <typename Index, typename Body>
auto for_all(Index index, Body body)
{
    return for_all_multi(sequence(index), body);
}
}
}
}

#endif
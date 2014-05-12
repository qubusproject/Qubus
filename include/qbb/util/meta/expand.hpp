#ifndef EXPAND_HPP
#define EXPAND_HPP

struct expand
{
    template <typename... Expressions>
    explicit expand(Expressions...)
    {
    }
};

#endif
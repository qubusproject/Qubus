#ifndef QBB_QUBUS_PATTERN_MEMBER_ACCESS_HPP
#define QBB_QUBUS_PATTERN_MEMBER_ACCESS_HPP

#include <qbb/qubus/IR/member_access_expr.hpp>
#include <qbb/qubus/pattern/variable.hpp>

#include <utility>
#include <functional>

inline namespace qbb
{
namespace qubus
{
namespace pattern
{
template <typename Object, typename MemberName>
class member_access_pattern
{
public:
    member_access_pattern(Object object_, MemberName member_name_)
    : object_(std::move(object_)), member_name_(std::move(member_name_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const member_access_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<member_access_expr>())
        {
            if (member_name_.match(concret_value->member_name()))
            {
                if (object_.match(concret_value->object()))
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
        object_.reset();
        member_name_.reset();
    }

private:
    Object object_;
    MemberName member_name_;
};

template <typename Object, typename MemberName>
member_access_pattern<Object, MemberName> member_access(Object object, MemberName member_name)
{
    return member_access_pattern<Object, MemberName>(object, member_name);
}

}
}
}

#endif

#ifndef QBB_QUBUS_STRUCT_HPP_H
#define QBB_QUBUS_STRUCT_HPP_H

#include <qbb/qubus/object.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/handle.hpp>

#include <vector>
#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
{

class struct_ final : public basic_object
{
public:
    explicit struct_(type struct_type_, std::vector<std::unique_ptr<object>> members_)
    : object_type_(std::move(struct_type_)), members_(std::move(members_))
    {
    }

    virtual ~struct_()
    {
        destruct();
    }

    const std::vector<std::unique_ptr<object>>& members() const
    {
        return members_;
    }

    type object_type() const override
    {
        return object_type_;
    }

    util::handle id() const override
    {
        return util::handle_from_ptr(this);
    }

    unsigned long int tag() const override
    {
        return 2;
    }

private:
    type object_type_;
    std::vector<std::unique_ptr<object>> members_;
};

}
}

#endif

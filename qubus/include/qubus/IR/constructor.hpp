#ifndef QUBUS_IR_CONSTRUCTOR_HPP
#define QUBUS_IR_CONSTRUCTOR_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/member.hpp>

#include <qubus/util/assert.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace qubus
{
class object_type;

class constructor
{
public:
    struct initializer
    {
    public:
        explicit initializer()
        {

        }

        const property& member() const
        {
            QUBUS_ASSERT(member_ != nullptr, "Invalid object state.");

            return *member_;
        }

        const std::vector<std::unique_ptr<expression>>& arguments() const
        {
            return arguments_;
        }
    private:
        const property* member_;
        std::vector<std::unique_ptr<expression>> arguments_;
    };

    constructor() = default;

    explicit constructor(const object_type& parent_type_, std::vector<initializer> initializers_,
                         std::unique_ptr<expression> body_);

    const object_type& parent_type() const
    {
        return *parent_type_;
    }

    const std::vector<initializer>& initializers() const
    {
        return initializers_;
    }

    const expression& body() const
    {
        QUBUS_ASSERT(body_ != nullptr, "Invalid object state.");

        return *body_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& parent_type_;
        ar& initializers_;
        ar& body_;
    }
private:
    const object_type* parent_type_;
    std::vector<initializer> initializers_;
    std::unique_ptr<expression> body_;
};

} // namespace qubus

#endif

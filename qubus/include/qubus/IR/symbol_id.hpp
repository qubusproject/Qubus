#ifndef QUBUS_IR_SYMBOL_ID_HPP
#define QUBUS_IR_SYMBOL_ID_HPP

#include <qubus/exception.hpp>

#include <qubus/util/hash.hpp>
#include <qubus/util/unused.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <functional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace qubus
{
class symbol_id_parsing_error : public virtual exception, public virtual std::logic_error
{
public:
    explicit symbol_id_parsing_error(std::string what_) : std::logic_error(std::move(what_))
    {
    }
};

class symbol_id
{
public:
    symbol_id() = default;
    explicit symbol_id(const std::string& id);

    explicit symbol_id(std::vector<std::string> components_);

    auto components() const
    {
        return components_ | boost::adaptors::transformed(
                                 [](const std::string& value) { return std::string_view(value); });
    }

    symbol_id get_prefix() const;
    const std::string& suffix() const;

    std::string string() const;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& components_;
    }

private:
    std::vector<std::string> components_;
};

bool operator==(const symbol_id& lhs, const symbol_id& rhs);
bool operator!=(const symbol_id& lhs, const symbol_id& rhs);

std::ostream& operator<<(std::ostream& out, const symbol_id& value);
}

namespace std
{

template <>
struct hash<qubus::symbol_id>
{
    using argument_type = qubus::symbol_id;
    using result_type = std::size_t;

    result_type operator()(const argument_type& s) const noexcept
    {
        std::size_t seed = 0;

        for (const auto& component : s.components())
        {
            qubus::util::hash_combine(seed, component);
        }

        return seed;
    }
};
}

#endif

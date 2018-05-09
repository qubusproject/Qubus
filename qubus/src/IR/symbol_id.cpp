#include <qubus/IR/symbol_id.hpp>

#include <boost/range/adaptor/sliced.hpp>
#include <boost/spirit/home/qi.hpp>

#include <qubus/util/assert.hpp>

namespace qubus
{
symbol_id::symbol_id(const std::string& id)
{
    namespace qi = boost::spirit::qi;
    using qi::ascii::alpha;
    using qi::ascii::alnum;

    auto first = id.cbegin();
    auto last = id.cend();

    using iterator_type = decltype(first);

    qi::rule<iterator_type, std::string()> qubus_id;
    qubus_id %= qi::raw[alpha >> *(alnum | '_')];

    bool r = qi::parse(first, last, qubus_id % '.', this->components_);

    if (!r || first != last)
        throw symbol_id_parsing_error("Unable to parse symbol id.");
}

symbol_id::symbol_id(std::vector<std::string> components_)
: components_(std::move(components_))
{
}

symbol_id symbol_id::get_prefix() const
{
    if (components_.size() < 2)
        return symbol_id();

    std::vector<std::string> prefix_components;

    for (const auto& component : components_ | boost::adaptors::sliced(0, components_.size() - 1))
    {
        prefix_components.push_back(component);
    }

    return symbol_id(std::move(prefix_components));
}

const std::string& symbol_id::suffix() const
{
    QUBUS_ASSERT(!components_.empty(), "Symbol ID has no valid suffix.");

    return components_.back();
}

std::string symbol_id::string() const
{
    std::string result;

    for (auto iter = components_.begin(), last = components_.end(); iter != last; ++iter)
    {
        result += *iter;

        if (iter != last - 1)
        {
            result += '.';
        }
    }

    return result;
}

bool operator==(const symbol_id& lhs, const symbol_id& rhs)
{
    return lhs.components() == rhs.components();
}

bool operator!=(const symbol_id& lhs, const symbol_id& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const symbol_id& value)
{
    out << value.string();

    return out;
}
}

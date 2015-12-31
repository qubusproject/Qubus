#include <qbb/qubus/IR/intrinsic_function_table.hpp>

#include <map>
#include <algorithm>
#include <memory>
#include <utility>
#include <mutex>

namespace qbb
{
namespace qubus
{

namespace
{

class intrinsic_function_info
{
public:
    explicit intrinsic_function_info(type result_type_) : result_type_{std::move(result_type_)}
    {
    }

    const type& result_type() const
    {
        return result_type_;
    }

private:
    type result_type_;
};

class overload_set
{
public:
    void add_overload(std::vector<type> arg_types, intrinsic_function_info function_info)
    {
        overloads_.emplace_back(std::move(arg_types), std::move(function_info));
    }

    const intrinsic_function_info& find_match(const std::vector<type>& arg_types) const
    {
        auto iter =
            std::find_if(begin(overloads_), end(overloads_),
                         [&](const std::pair<std::vector<type>, intrinsic_function_info>& value)
                         {
                             return value.first == arg_types;
                         });

        if (iter == end(overloads_))
            throw 0;

        return iter->second;
    }

private:
    std::vector<std::pair<std::vector<type>, intrinsic_function_info>> overloads_;
};

class intrinsic_function_table
{
public:
    intrinsic_function_table()
    {
        const char* math_functions[] = {"sin", "cos", "tan", "exp", "abs", "sqrt"};
        type real_types[] = {types::double_{}, types::float_{}};
        type math_types[] = {types::double_{}, types::float_{}, types::complex(types::double_{}),
                             types::complex(types::float_{})};

        for (auto function : math_functions)
        {
            for (const auto& t : real_types)
            {
                add_intrinsic_function(function, {t}, t);
            }
        }

        for (const auto& t : real_types)
        {
            add_intrinsic_function("pow", {t, types::integer{}}, t);
        }

        for (const auto& t : math_types)
        {
            add_intrinsic_function("extent", {types::tensor(t), types::integer{}},
                                   types::integer{});
            add_intrinsic_function("extent", {types::array(t), types::integer{}}, types::integer{});
            add_intrinsic_function("extent", {types::sparse_tensor(t), types::integer{}},
                                   types::integer{});
        }

        add_intrinsic_function("extent", {types::array(types::integer{}), types::integer{}},
                               types::integer{});

        add_intrinsic_function("delta", {types::integer{}, types::integer{}}, types::integer{});

        add_intrinsic_function("min", {types::integer{}, types::integer{}}, types::integer{});
        add_intrinsic_function("max", {types::integer{}, types::integer{}}, types::integer{});

        add_intrinsic_function("select", {types::bool_{}, types::integer{}, types::integer{}},
                               types::integer{});
    }

    const type& lookup_result_type(const std::string& name,
                                   const std::vector<type>& arg_types) const
    {
        const overload_set& os = lookup_overload_set(name);

        return os.find_match(arg_types).result_type();
    }

private:
    void add_intrinsic_function(std::string name, std::vector<type> arg_types, type result_type)
    {
        auto iter = overload_sets_.find(name);

        if (iter == overload_sets_.end())
            iter = overload_sets_.emplace(std::move(name), overload_set()).first;

        overload_set& os = iter->second;

        os.add_overload(std::move(arg_types), intrinsic_function_info(std::move(result_type)));
    }

    const overload_set& lookup_overload_set(const std::string& name) const
    {
        auto iter = overload_sets_.find(name);

        if (iter == overload_sets_.end())
            throw 0;

        return iter->second;
    }

    overload_set& lookup_overload_set(const std::string& name)
    {
        auto iter = overload_sets_.find(name);

        if (iter == overload_sets_.end())
            throw 0;

        return iter->second;
    }

    std::map<std::string, overload_set> overload_sets_;
};

std::unique_ptr<intrinsic_function_table> intrinsic_function_table_ = {};

void init_intrinsic_function_table() noexcept
{
    intrinsic_function_table_.reset(new intrinsic_function_table());
}

std::once_flag intrinsic_function_table_init_flag = {};
}

type lookup_intrinsic_result_type(const std::string& name, const std::vector<type>& arg_types)
{
    std::call_once(intrinsic_function_table_init_flag, init_intrinsic_function_table);

    return intrinsic_function_table_->lookup_result_type(name, arg_types);
}
}
}
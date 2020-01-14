#ifndef QUBUS_MODULE_HPP
#define QUBUS_MODULE_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function.hpp>
#include <qubus/IR/symbol_id.hpp>
#include <qubus/IR/templates.hpp>
#include <qubus/IR/types.hpp>
#include <qubus/IR/variable_declaration.hpp>
#include <qubus/exception.hpp>

#include <hpx/include/serialization.hpp>

#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/map.hpp>

#include <qubus/util/unused.hpp>

#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace qubus
{
class duplicate_symbol_error : public virtual exception, public virtual std::runtime_error
{
public:
    duplicate_symbol_error(symbol_id module_id_, std::string symbol_);

    const symbol_id& module_id() const
    {
        return module_id_;
    }

    const std::string& symbol() const
    {
        return symbol_;
    }

private:
    symbol_id module_id_;
    std::string symbol_;
};

class module_import_error : public virtual exception,
                            public virtual std::runtime_error,
                            public virtual std::nested_exception
{
public:
    module_import_error(symbol_id module_id_, std::string imported_module_id_);

    const symbol_id& module_id() const
    {
        return module_id_;
    }

    const std::string& imported_module_id() const
    {
        return imported_module_id_;
    }

private:
    symbol_id module_id_;
    std::string imported_module_id_;
};

class unknown_symbol_error : public virtual exception, public virtual std::runtime_error
{
public:
    unknown_symbol_error(symbol_id module_id_, std::string symbol_);

    const symbol_id& module_id() const
    {
        return module_id_;
    }

    const std::string& symbol() const
    {
        return symbol_;
    }

private:
    symbol_id module_id_;
    std::string symbol_;
};

class module
{
public:
    explicit module(symbol_id id_);

    module(const module& other) = delete;
    module(module&&) = default;

    module& operator=(const module& other) = delete;
    module& operator=(module&&) = default;

    const symbol_id& id() const
    {
        return id_;
    }

    const std::vector<symbol_id>& imports() const
    {
        return m_imports;
    }

    auto functions() const
    {
        return function_index_ | boost::adaptors::map_values | boost::adaptors::indirected;
    }

    auto types() const
    {
        return type_index_ | boost::adaptors::map_values;
    }

    auto type_templates() const
    {
        return type_templates_ | boost::adaptors::map_values;
    }

    auto function_templates() const
    {
        return function_templates_ | boost::adaptors::map_values;
    }

    const function& lookup_function(const std::string& id) const;

    const type_template& lookup_type_template(const std::string& id) const;
    const function_template& lookup_function_template(const std::string& id) const;

    void add_function(std::string name,
                      std::vector<std::shared_ptr<const variable_declaration>> params,
                      std::shared_ptr<const variable_declaration> result,
                      std::unique_ptr<expression> body);
    void add_type(object_type user_defined_type);

    template <typename Range>
    void add_types(const Range& types)
    {
        for (const auto& type : types)
        {
            add_type(type);
        }
    }

    void dump(std::ostream& out) const;

    void save(hpx::serialization::output_archive& ar, unsigned version) const;
    void load(hpx::serialization::input_archive& ar, unsigned version);

    HPX_SERIALIZATION_SPLIT_MEMBER();

private:
    symbol_id id_;

    std::vector<symbol_id> m_imports;

    std::unordered_map<std::string, std::unique_ptr<function>> function_index_;
    std::unordered_map<std::string, type> type_index_;
    std::unordered_map<std::string, type_template> type_templates_;
    std::unordered_map<std::string, function_template> function_templates_;

    module() = default;

    friend void load_construct_data(hpx::serialization::input_archive& ar, module* mod,
                                    unsigned version);
};

std::unique_ptr<module> clone(const module& other);
} // namespace qubus

#endif

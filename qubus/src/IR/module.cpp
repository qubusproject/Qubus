#include <qubus/IR/module.hpp>

#include <qubus/IR/parsing.hpp>
#include <qubus/IR/pretty_printer.hpp>

namespace qubus
{

duplicate_symbol_error::duplicate_symbol_error(symbol_id module_id_, std::string symbol_)
: std::runtime_error("The symbol " + symbol_ + " has already be defined in module " +
                     module_id_.string() + "."),
  module_id_(std::move(module_id_)),
  symbol_(std::move(symbol_))
{
}

module_import_error::module_import_error(symbol_id module_id_, std::string imported_module_id_)
: std::runtime_error("Failed to import module " + imported_module_id_ + " into " +
                     module_id_.string() + "."),
  module_id_(std::move(module_id_)),
  imported_module_id_(std::move(imported_module_id_))
{
}

unknown_symbol_error::unknown_symbol_error(symbol_id module_id_, std::string symbol_)
: std::runtime_error("The symbol " + symbol_ + " is not defined in module " + module_id_.string() +
                     "."),
  module_id_(std::move(module_id_)),
  symbol_(std::move(symbol_))
{
}

module::module(symbol_id id_) : id_(std::move(id_))
{
}

const function& module::lookup_function(const std::string& id) const
{
    auto search_result = function_index_.find(id);

    if (search_result != function_index_.end())
    {
        return *search_result->second;
    }
    else
    {
        throw unknown_symbol_error(this->id(), id);
    }
}

void module::add_function(std::string name,
                          std::vector<std::shared_ptr<const variable_declaration>> params,
                          std::shared_ptr<const variable_declaration> result,
                          std::unique_ptr<expression> body)
{
    if (type_index_.count(name) > 0)
        throw duplicate_symbol_error(id(), name);

    auto func = std::make_unique<function>(*this, name, std::move(params), std::move(result),
                                           std::move(body));

    auto [pos, has_been_added] = function_index_.emplace(name, std::move(func));

    if (!has_been_added)
        throw duplicate_symbol_error(id(), name);
}

void module::add_type(type new_type)
{
    std::string type_id = std::string(new_type.name());

    if (function_index_.count(type_id) > 0)
        throw duplicate_symbol_error(id(), type_id);

    auto [pos, has_been_added] = type_index_.emplace(type_id, std::move(new_type));

    if (!has_been_added)
        throw duplicate_symbol_error(id(), type_id);
}

void module::dump(std::ostream& out) const
{
    auto serialized_code = pretty_print(*this);

    auto target = carrot::get_file_target();

    carrot::plain_form form(target);

    carrot::render(serialized_code, form);

    out << form.to_string();
}

void module::save(hpx::serialization::output_archive& ar, unsigned QUBUS_UNUSED(version)) const
{
    auto serialized_code = pretty_print(*this);

    auto target = carrot::get_file_target();

    carrot::plain_form form(target);

    carrot::render(serialized_code, form);

    ar& form.to_string();
}

void module::load(hpx::serialization::input_archive& ar, unsigned QUBUS_UNUSED(version))
{
    std::string code;

    ar& code;

    auto parsed_code = parse_qir(code);

    *this = std::move(*parsed_code);
}

void load_construct_data(hpx::serialization::input_archive& ar, module* mod,
                         unsigned QUBUS_UNUSED(version))
{
    ::new (mod) module();
}

std::unique_ptr<module> clone(const module& other)
{
    auto new_module = std::make_unique<module>(other.id());

    for (const auto& function : other.functions())
    {
        new_module->add_function(function.name(), function.params(), function.result(),
                                 clone(function.body()));
    }

    for (const auto& type : other.types())
    {
        new_module->add_type(type);
    }

    return new_module;
}
}
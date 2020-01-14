#include <qubus/IR/module_loader.hpp>

#include <qubus/IR/qir.hpp>

#include <qubus/util/assert.hpp>

#include <stack>
#include <string_view>

namespace qubus
{

namespace
{

class symbol_table
{
public:
    explicit symbol_table() = default;

    explicit symbol_table(std::shared_ptr<const symbol_table> parent_table)
    : m_parent_table(std::move(parent_table))
    {
    }

    std::shared_ptr<const variable_declaration> create_variable(std::string name, type datatype)
    {
        auto var = std::make_shared<variable_declaration>(name, std::move(datatype));

        auto [iter, inserted] = m_symbol_table.emplace(std::move(name), std::move(var));

        if (!inserted)
            throw util::exception("Duplicate definition.");

        return std::static_pointer_cast<const variable_declaration>(iter->second);
    }

    [[nodiscard]] std::shared_ptr<const variable_declaration> lookup_variable(const ast::variable& var) const
    {
        auto search_result = m_symbol_table.find(var.name);

        if (search_result == m_symbol_table.end())
        {
            if (m_parent_table)
            {
                return m_parent_table->lookup_variable(var);
            }

            return nullptr;
        }

        if (auto var = std::dynamic_pointer_cast<const variable_declaration>(search_result->second))
        {
            return var;
        }

        throw util::exception("The symbol is of the wrong type.");
    }
private:
    using symbol_data_type = std::shared_ptr<const definition>;

    std::unordered_map<std::string, symbol_data_type> m_symbol_table;

    std::shared_ptr<const symbol_table> m_parent_table;
};

class assembly_unit
{
public:
    function instantiate_function_template(const symbol_id& id);
    type instantiate_type_template(const symbol_id& id);

    [[nodiscard]] const module& parent_module() const
    {
        QUBUS_ASSERT(m_parent_module != nullptr, "Invalid object.");

        return *m_parent_module;
    }
private:
    const module* m_parent_module;
};

struct loading_session
{
    loading_session() = default;

    assembly constructed_assembly;

    std::unordered_map<symbol_id, assembly_unit> known_assembly_units;
};

void reify_function(function& func, symbol_table& sym_table)
{
    pattern::variable<const expression&> scope;
    pattern::variable<symbol_id> id;

    auto open_lexical_scope = pattern::make_matcher<expression, void>()
        .case_(lexical_scope(), [&]
        {

        });

    auto close_lexical_scope = pattern::make_matcher<expression, void>()
        .case_(lexical_scope(), [&]
        {

        });

    auto substitute_object_references = pattern::make_matcher<expression, std::unique_ptr<expression>>()
        .case_(symbol(id), [&]
        {

        });
}

void reify_type(type& t, symbol_table& sym_table)
{

}

void load_module(const module& mod, const module_resolver& mod_resolver, loading_session& session)
{
    // Create a new symbol table for this module.
    // Every module only can observe any symbols it itself defines
    // or any which are imported from other modules.
    auto sym_table = std::make_shared<symbol_table>();

    // Create a new assembly unit for this module.
    session.known_assembly_units.emplace(mod.id(), assembly_unit());

    // Import the implicitly imported core module.
    if (session.known_assembly_units.count(symbol_id("core")) > 0)
    {
        // Get the core module.
        const module& core_module = mod_resolver.lookup_module(symbol_id("core"));

        // Load the core module.
        load_module(core_module, mod_resolver, session);
    }

    // Import all dependencies.
    for (const symbol_id& import : mod.imports())
    {
        // Load the dependency.
        if (session.known_assembly_units.count(import) > 0)
        {
            // Get the module for the import.
            const module& imported_mod = mod_resolver.lookup_module(import);

            // Load the module.
            load_module(imported_mod, mod_resolver, session);
        }
    }

    // Construct the assembly unit for this module.

    // Load all symbols which are imported by the module
    // into the symbol table.

    

    // Reify all functions, types and templates.

    for (const function& func : mod.functions())
    {
        function reified_function = clone(func);

        reify_function(reified_function, *sym_table);
    }

    for (const type& t : mod.types())
    {
        type reified_type = clone(t);

        reify_type(reified_type, *sym_table);
    }
}

class module_loader_implementation final : public module_loader
{
public:
    [[nodiscard]] assembly load(const module& interface_module, const module_resolver& mod_resolver) const final
    {
        // Create a new loading session.
        loading_session session;

        load_module(interface_module, mod_resolver, session);

        return session.constructed_assembly;
    }
};

}

std::unique_ptr<module_loader> create_module_loader()
{
    return std::make_unique<module_loader_implementation>();
}

}
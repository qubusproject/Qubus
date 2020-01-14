#include <qubus/basic_address_space.hpp>

#include <utility>

namespace qubus
{

void basic_address_space::add_subspace(std::unique_ptr<virtual_address_space> subspace)
{
    subspaces_.push_back(std::move(subspace));
}

hpx::future<instance_token> basic_address_space::resolve_object(object_id id)
{
    std::vector<hpx::future<instance_token>> search_results;
    search_results.reserve(subspaces_.size());

    for (const auto& subspace : subspaces_)
    {
        search_results.push_back(subspace->resolve_object(id));
    }

    auto search_result = hpx::when_any(search_results);

    return search_result.then([] (auto search_result){
        auto search_result_val = search_result.get();

        return search_result_val.futures[search_result_val.index].get();
    });
}

hpx::future<void> basic_address_space::invalidate_object(object_id id)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(subspaces_.size());

    for (const auto& subspace : subspaces_)
    {
        futures.push_back(subspace->invalidate_object(id));
    }

    return hpx::when_all(std::move(futures));
}

hpx::future<void> basic_address_space::free_object(object_id id)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(subspaces_.size());

    for (const auto& subspace : subspaces_)
    {
        futures.push_back(subspace->free_object(id));
    }

    return hpx::when_all(std::move(futures));
}

} // namespace qubus
#include <qubus/dataflow.hpp>

#include <qubus/local_runtime.hpp>

#include <hpx/parallel/executors.hpp> // Workaround for missing includes.

using server_type = hpx::components::component<qubus::distributed_access_token_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_distributed_access_token_server);

namespace qubus
{

class dataflow_event
{
public:
    ~dataflow_event()
    {
        p_.set_value();
    }

    hpx::future<void> get_future()
    {
        return p_.get_future();
    }

private:
    hpx::promise<void> p_;
};

access_token::access_token() : event_(std::make_unique<dataflow_event>())
{
}

access_token::~access_token() = default;

access_token::access_token(access_token&&) noexcept = default;

access_token& access_token::operator=(access_token&&) noexcept = default;

hpx::future<void> access_token::get_future()
{
    return event_->get_future();
}

dataflow_event* access_token::release() &&
{
    return event_.release();
}

shared_access_token::shared_access_token(access_token&& token) : event_(std::move(token).release())
{
}

distributed_access_token_server::distributed_access_token_server(access_token&& token)
: internal_token_(std::move(token))
{
}

distributed_access_token_server::distributed_access_token_server(shared_access_token token)
: internal_token_(std::move(token))
{
}

distributed_access_token::distributed_access_token(access_token&& token)
: base_type(hpx::local_new<distributed_access_token_server>(std::move(token)))
{
}

distributed_access_token::distributed_access_token(shared_access_token token)
: base_type(hpx::local_new<distributed_access_token_server>(std::move(token)))
{
}

void dataflow_graph::finalize()
{
    for (const auto& [id, future] : wavefront_map_)
    {
        future.get();
    }
}

hpx::future<access_token> dataflow_graph::schedule_modification(const object& obj)
{
    auto search_result = wavefront_map_.find(obj.id());

    access_token token;

    if (search_result != wavefront_map_.end())
    {
        auto f = token.get_future();

        auto ready_token = search_result->second.then(
                get_local_runtime().get_service_executor(), [token = std::move(token)](
                        const hpx::shared_future<void>&
                        value) mutable {
                    value.get();

                    return std::move(token);
                });

        search_result->second = std::move(f);

        return ready_token;
    }
    else
    {
        wavefront_map_.emplace(obj.id(), token.get_future());

        return hpx::make_ready_future(std::move(token));
    }
}

hpx::future<access_token> dataflow_graph::schedule_read(const object& obj)
{
    return schedule_modification(obj);
}
}
#ifndef QUBUS_DATAFLOW_HPP
#define QUBUS_DATAFLOW_HPP

#include <qubus/object.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/lcos/future.hpp>

#include <memory>
#include <unordered_map>
#include <utility>

namespace qubus
{

class dataflow_event;

class access_token
{
public:
    access_token();

    access_token(const access_token&) = delete;
    access_token(access_token&&) noexcept;

    access_token& operator=(const access_token&) = delete;
    access_token& operator=(access_token&&) noexcept;

    ~access_token();

    [[nodiscard]] hpx::future<void> get_future();
    dataflow_event* release() &&;

private:
    std::unique_ptr<dataflow_event> event_;
};

class shared_access_token
{
public:
    shared_access_token(access_token&& token);

    shared_access_token(const shared_access_token&) = default;
    shared_access_token(shared_access_token&&) = default;

    shared_access_token& operator=(const shared_access_token&) = default;
    shared_access_token& operator=(shared_access_token&&) = default;

private:
    std::shared_ptr<dataflow_event> event_;
};

class distributed_access_token_server
: public hpx::components::component_base<distributed_access_token_server>
{
public:
    explicit distributed_access_token_server(access_token&& token);
    explicit distributed_access_token_server(shared_access_token token);
private:
    shared_access_token internal_token_;
};

class distributed_access_token
: public hpx::components::client_base<distributed_access_token, distributed_access_token_server>
{
public:
    using base_type = hpx::components::client_base<distributed_access_token, distributed_access_token_server>;

    distributed_access_token() = default;
    distributed_access_token(access_token&& token);
    distributed_access_token(shared_access_token token);
};

class dataflow_graph
{
public:
    void finalize();

    [[nodiscard]] hpx::future<access_token> schedule_modification(const object& obj);
    [[nodiscard]] hpx::future<access_token> schedule_read(const object& obj);

private:
    // TODO: Substitute this with a dense map.
    std::unordered_map<object_id, hpx::shared_future<void>> wavefront_map_;
};
}

#endif

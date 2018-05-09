#include <hpx/config.hpp>

#include <qubus/performance_models/regression_performance_model.hpp>

#include <qubus/performance_models/symbolic_regression.hpp>

#include <qubus/get_view.hpp>
#include <qubus/host_object_views.hpp>

#include <hpx/include/local_lcos.hpp>

#include <qubus/util/assert.hpp>

#include <map>
#include <mutex>
#include <utility>
#include <vector>

namespace qubus
{

namespace
{

class regression_analysis
{
public:
    regression_analysis() : regression_model_(1000)
    {
    }

    regression_analysis(const regression_analysis&) = delete;
    regression_analysis& operator=(const regression_analysis&) = delete;

    regression_analysis(regression_analysis&&) = default;
    regression_analysis& operator=(regression_analysis&&) = default;

    void add_datapoint(std::vector<double> arguments, std::chrono::microseconds execution_time)
    {
        using namespace std::chrono_literals;

        regression_model_.add_datapoint(std::move(arguments), std::move(execution_time));

        ++number_of_new_data_points_;

        if (regression_model_.size_of_dataset() < 10)
            return;

        auto accuracy = regression_model_.accuracy();

        constexpr auto target_accuarcy = 20us;
        constexpr auto minimal_accuary = 100us;

        static_assert(target_accuarcy < minimal_accuary,
                      "The target accuarcy should always be smaller than the minimal accuarcy.");

        if (accuracy > minimal_accuary)
        {
            update();
            return;
        }

        if (number_of_new_data_points_ >= 10)
        {
            accuracy = regression_model_.update_cheaply();

            if (accuracy > target_accuarcy)
            {
                update();
            }

            return;
        }
    }

    boost::optional<std::chrono::microseconds> query(const std::vector<double>& arguments) const
    {
        return regression_model_.query(arguments);
    }

    boost::optional<std::chrono::microseconds> accuracy() const
    {
        return regression_model_.accuracy();
    }

private:
    void update()
    {
        regression_model_.update();

        number_of_new_data_points_ = 0;
    }

    symbolic_regression regression_model_;

    long int number_of_new_data_points_ = 0;
};

std::vector<hpx::naming::gid_type> generate_key_from_ctx(const execution_context& ctx)
{
    std::vector<hpx::naming::gid_type> key;
    key.reserve(ctx.args().size());

    for (const auto& obj : ctx.args())
    {
        const auto& datatype = obj.object_type();

        if (!(datatype == types::integer{} || datatype == types::float_{} ||
              datatype == types::double_{}))
        {
            key.push_back(obj.get_raw_gid());
        }
    }

    return key;
}

std::vector<double> extract_arguments_from_ctx(const execution_context& ctx,
                                               address_space& host_addr_space)
{
    std::vector<double> arguments;
    arguments.reserve(ctx.args().size());

    for (const auto& obj : ctx.args())
    {
        auto datatype = obj.object_type();

        if (datatype == types::integer{})
        {
            auto view =
                get_view_for_locked_object<host_scalar_view<util::index_t>>(obj, host_addr_space).get();

            arguments.push_back(view.get());
        }
        else if (datatype == types::double_{})
        {
            auto view = get_view_for_locked_object<host_scalar_view<double>>(obj, host_addr_space).get();

            arguments.push_back(view.get());
        }
        else if (datatype == types::float_{})
        {
            auto view = get_view_for_locked_object<host_scalar_view<float>>(obj, host_addr_space).get();

            arguments.push_back(view.get());
        }
    }

    return arguments;
}
} // namespace

class regression_performance_model_impl
{
public:
    explicit regression_performance_model_impl(address_space& host_addr_space_)
    : host_addr_space_(&host_addr_space_)
    {
    }

    void sample_execution_time(const symbol_id& QUBUS_UNUSED(func), const execution_context& ctx,
                               std::chrono::microseconds execution_time)
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(regression_analyses_mutex_);

        auto key = generate_key_from_ctx(ctx);
        auto arguments = extract_arguments_from_ctx(ctx, *host_addr_space_);

        auto search_result = regression_analyses_.find(key);

        if (search_result == regression_analyses_.end())
        {
            search_result = regression_analyses_.emplace(key, regression_analysis()).first;
        }

        search_result->second.add_datapoint(std::move(arguments), std::move(execution_time));
    }

    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& QUBUS_UNUSED(func),
                                const execution_context& ctx) const
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(regression_analyses_mutex_);

        auto key = generate_key_from_ctx(ctx);

        auto search_result = regression_analyses_.find(key);

        if (search_result != regression_analyses_.end())
        {
            auto arguments = extract_arguments_from_ctx(ctx, *host_addr_space_);

            auto query_result = search_result->second.query(arguments);
            auto accuracy_result = search_result->second.accuracy();

            if (query_result && accuracy_result)
            {
                return performance_estimate{*std::move(query_result), *std::move(accuracy_result)};
            }
            else
            {
                return boost::none;
            }
        }
        else
        {
            return boost::none;
        }
    }

private:
    address_space* host_addr_space_;

    mutable hpx::lcos::local::mutex regression_analyses_mutex_;
    std::map<std::vector<hpx::naming::gid_type>, regression_analysis> regression_analyses_;
};

regression_performance_model::regression_performance_model(address_space& host_addr_space_)
: impl_(std::make_unique<regression_performance_model_impl>(host_addr_space_))
{
}

regression_performance_model::~regression_performance_model() = default;

void regression_performance_model::sample_execution_time(const symbol_id& func,
                                                         const execution_context& ctx,
                                                         std::chrono::microseconds execution_time)
{
    QUBUS_ASSERT(impl_, "Uninitialized object.");

    impl_->sample_execution_time(func, ctx, std::move(execution_time));
}

boost::optional<performance_estimate>
regression_performance_model::try_estimate_execution_time(const symbol_id& func,
                                                          const execution_context& ctx) const
{
    QUBUS_ASSERT(impl_, "Uninitialized object.");

    return impl_->try_estimate_execution_time(func, ctx);
}
} // namespace qubus

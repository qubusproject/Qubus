#ifndef QUBUS_PASS_MANAGER_HPP
#define QUBUS_PASS_MANAGER_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function.hpp>

#include <qubus/isl/context.hpp>

#include <boost/any.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/optional_ref.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace qubus
{

class pass_resource_manager
{
public:
    isl::context& get_isl_ctx();

private:
    isl::context isl_ctx_;
};

using analysis_id = std::type_index;

template <typename AnalysisPass>
analysis_id get_analysis_id()
{
    return typeid(AnalysisPass);
}

using preserved_analyses_info = std::vector<analysis_id>;

class analysis_manager;

template <typename Analysis>
struct analysis_traits
{
    using result_type = typename Analysis::result_type;
};

class analysis_result
{
public:
    analysis_result() = default;

    template <typename T>
    analysis_result(T value_)
    : self_(std::make_unique<analysis_result_wrapper<T>>(std::move(value_)))
    {
    }

    analysis_result(const analysis_result&) = delete;
    analysis_result& operator=(const analysis_result&) = delete;

    analysis_result(analysis_result&&) = default;
    analysis_result& operator=(analysis_result&&) = default;

    bool empty() const
    {
        return self_ == nullptr;
    }

    template <typename T>
    const T& as() const
    {
        auto typed_self = dynamic_cast<const analysis_result_wrapper<T>*>(self_.get());

        QUBUS_ASSERT(typed_self != nullptr, "Unexpected analysis result type.");

        return typed_self->value();
    }

    template <typename T>
    T& as()
    {
        auto typed_self = dynamic_cast<analysis_result_wrapper<T>*>(self_.get());

        QUBUS_ASSERT(typed_self != nullptr, "Unexpected analysis result type.");

        return typed_self->value();
    }

private:
    class analysis_result_interface
    {
    public:
        analysis_result_interface() = default;

        virtual ~analysis_result_interface() = default;

        analysis_result_interface(const analysis_result_interface&) = delete;
        analysis_result_interface(analysis_result_interface&&) = delete;

        analysis_result_interface& operator=(const analysis_result_interface&) = delete;
        analysis_result_interface& operator=(analysis_result_interface&&) = delete;
    };

    template <typename T>
    class analysis_result_wrapper final : public analysis_result_interface
    {
    public:
        virtual ~analysis_result_wrapper() = default;

        explicit analysis_result_wrapper(T value_) : value_(std::move(value_))
        {
        }

        const T& value() const
        {
            return value_;
        }

        T& value()
        {
            return value_;
        }

    private:
        T value_;
    };

    std::unique_ptr<analysis_result_interface> self_;
};

class analysis_pass
{
public:
    template <typename AnalysisPass>
    analysis_pass(AnalysisPass analysis_)
    : self_(std::make_unique<analysis_pass_wrapper<AnalysisPass>>(std::move(analysis_)))
    {
    }

    analysis_result run(const expression& expr, analysis_manager& manager,
                        pass_resource_manager& resource_manager_) const
    {
        return self_->run(expr, manager, resource_manager_);
    }

    analysis_id id() const
    {
        return self_->id();
    }

    std::vector<analysis_id> required_analyses() const
    {
        return self_->required_analyses();
    }

private:
    class analysis_pass_interface
    {
    public:
        analysis_pass_interface() = default;
        virtual ~analysis_pass_interface() = default;

        analysis_pass_interface(const analysis_pass_interface&) = delete;
        analysis_pass_interface(analysis_pass_interface&&) = delete;

        analysis_pass_interface& operator=(const analysis_pass_interface&) = delete;
        analysis_pass_interface& operator=(analysis_pass_interface&&) = delete;

        virtual analysis_result run(const expression& expr, analysis_manager& manager,
                                    pass_resource_manager& resource_manager_) const = 0;

        virtual std::vector<analysis_id> required_analyses() const = 0;

        virtual analysis_id id() const = 0;
    };

    template <typename AnalysisPass>
    class analysis_pass_wrapper final : public analysis_pass_interface
    {
    public:
        virtual ~analysis_pass_wrapper() = default;

        analysis_pass_wrapper(AnalysisPass analysis_) : analysis_(std::move(analysis_))
        {
        }

        analysis_result run(const expression& expr, analysis_manager& manager,
                            pass_resource_manager& resource_manager_) const override
        {
            return analysis_.run(expr, manager, resource_manager_);
        }

        std::vector<analysis_id> required_analyses() const override
        {
            return analysis_.required_analyses();
        }

        analysis_id id() const override
        {
            return typeid(AnalysisPass);
        }

    private:
        AnalysisPass analysis_;
    };

    std::unique_ptr<analysis_pass_interface> self_;
};

class analysis_pass_registry
{
public:
    template <typename AnalysisPass>
    void register_analysis_pass()
    {
        std::lock_guard<std::mutex> guard(analysis_pass_table_mutex_);

        analysis_pass_constructor_table_.emplace(get_analysis_id<AnalysisPass>(),
                                                 [] { return AnalysisPass(); });
    }

    std::vector<analysis_pass> construct_all_passes() const;

    static analysis_pass_registry& get_instance();
private:
    mutable std::mutex analysis_pass_table_mutex_;
    std::unordered_map<analysis_id, std::function<analysis_pass()>>
        analysis_pass_constructor_table_;
};

template <typename AnalysisPass>
struct register_analysis_pass
{
public:
    register_analysis_pass()
    {
        analysis_pass_registry::get_instance().register_analysis_pass<AnalysisPass>();
    }
};

#define QUBUS_REGISTER_ANALYSIS_PASS(ANALYSIS_PASS)                                                \
    register_analysis_pass<ANALYSIS_PASS> ANALYSIS_PASS##_qubus_pass_init = {}

class analysis_node
{
public:
    explicit analysis_node(analysis_pass pass_, analysis_manager& manager_,
                           pass_resource_manager& resource_manager_);

    void add_dependent(analysis_node& dependent);

    const analysis_pass& pass() const;

    template <typename Analysis>
    util::optional_ref<const typename analysis_traits<Analysis>::result_type>
    get_analysis_cached(const expression& expr) const
    {
        if (!cached_result_.empty())
        {
            const auto& typed_result =
                cached_result_.as<typename analysis_traits<Analysis>::result_type>();

            return typed_result;
        }
        else
        {
            return {};
        }
    }

    template <typename Analysis>
    const typename analysis_traits<Analysis>::result_type& get_analysis(const expression& expr)
    {
        if (cached_result_.empty())
        {
            cached_result_ = pass_.run(expr, manager_, *resource_manager_);
        }

        const auto& typed_result =
            cached_result_.as<typename analysis_traits<Analysis>::result_type>();

        return typed_result;
    }

    void invalidate();

private:
    analysis_pass pass_;
    analysis_result cached_result_;

    std::vector<analysis_node*> dependents_;

    std::reference_wrapper<analysis_manager> manager_;
    pass_resource_manager* resource_manager_;
};

class analysis_manager
{
public:
    explicit analysis_manager(pass_resource_manager& resource_manager_);

    template <typename Analysis>
    util::optional_ref<const typename analysis_traits<Analysis>::result_type>
    get_analysis_cached(const expression& expr) const
    {
        auto search_result = analysis_table_.find(get_analysis_id<Analysis>());

        if (search_result != analysis_table_.end())
        {
            return search_result->second->template get_analysis_cached<Analysis>(expr);
        }
        else
        {
            throw 0; // Unknown analysis
        }
    }

    template <typename Analysis>
    const typename analysis_traits<Analysis>::result_type& get_analysis(const expression& expr)
    {
        auto search_result = analysis_table_.find(get_analysis_id<Analysis>());

        if (search_result != analysis_table_.end())
        {
            return search_result->second->template get_analysis<Analysis>(expr);
        }
        else
        {
            throw 0; // Unknown analysis
        }
    }

    void invalidate(const preserved_analyses_info& preserved_analyses);
    void invalidate();
private:
    std::unordered_map<analysis_id, std::unique_ptr<analysis_node>> analysis_table_;
};

class transformation_pass
{
public:
    template <typename TransformationPass>
    transformation_pass(TransformationPass transformation_)
    : self_(std::make_unique<transformation_pass_wrapper<TransformationPass>>(
          std::move(transformation_)))
    {
    }

    preserved_analyses_info run(function& fun, analysis_manager& manager) const
    {
        return self_->run(fun, manager);
    }

private:
    class transformation_pass_interface
    {
    public:
        transformation_pass_interface() = default;
        virtual ~transformation_pass_interface() = default;

        transformation_pass_interface(const transformation_pass_interface&) = delete;
        transformation_pass_interface(transformation_pass_interface&&) = delete;

        transformation_pass_interface& operator=(const transformation_pass_interface&) = delete;
        transformation_pass_interface& operator=(transformation_pass_interface&&) = delete;

        virtual preserved_analyses_info run(function& fun,
                                            analysis_manager& manager) const = 0;
    };

    template <typename TransformationPass>
    class transformation_pass_wrapper final : public transformation_pass_interface
    {
    public:
        virtual ~transformation_pass_wrapper() = default;

        transformation_pass_wrapper(TransformationPass transformation_)
        : transformation_(std::move(transformation_))
        {
        }

        preserved_analyses_info run(function& fun,
                                    analysis_manager& manager) const override
        {
            return transformation_.run(fun, manager);
        }

    private:
        TransformationPass transformation_;
    };

    std::unique_ptr<transformation_pass_interface> self_;
};

class pass_manager
{
public:
    pass_manager();

    template <typename TransformationPass>
    void add_transformation(TransformationPass transformation)
    {
        optimization_pipeline_.emplace_back(std::move(transformation));
    }

    preserved_analyses_info run(function& fun);
private:
    pass_resource_manager resource_manager_;
    analysis_manager analysis_man_;
    std::vector<transformation_pass> optimization_pipeline_;
};
}

#endif

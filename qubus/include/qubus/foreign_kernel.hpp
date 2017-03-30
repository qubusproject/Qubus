#ifndef QUBUS_FOREIGN_KERNEL_HPP
#define QUBUS_FOREIGN_KERNEL_HPP

#include <qubus/local_runtime.hpp>
#include <qubus/runtime.hpp>

#include <qubus/host_object_views.hpp>
#include <qubus/associated_qubus_type.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/computelet.hpp>
#include <qubus/foreign_computelet.hpp>

#include <qubus/util/assert.hpp>

#include <functional>
#include <type_traits>

namespace qubus
{

template <typename... Parameters>
class foreign_kernel
{
public:
    foreign_kernel(std::string id_) : computelet_(std::move(id_), get_result_type(), get_parameter_types())
    {
    }

    template <typename Implementation>
    void add_version(architecture_identifier target, Implementation implementation)
    {
        computelet_.add_version(std::move(target), std::move(implementation));
    }

    void operator()(Parameters... parameters)
    {
        if (!thunk_.valid())
        {
            thunk_ = make_computelet(generate_thunk());
        }

        kernel_arguments kernel_args;

        boost::hana::for_each(boost::hana::make_tuple(std::ref(parameters)...),
                              [&kernel_args](auto param) {
                                  if (std::is_const<typename decltype(param)::type>::value)
                                  {
                                      kernel_args.push_back_arg(param.get().get_object());
                                  }
                                  else
                                  {
                                      kernel_args.push_back_result(param.get().get_object());
                                  }
                              });

        get_runtime().execute(thunk_, std::move(kernel_args)).get();
    }

private:
    template <typename T>
    static constexpr bool is_immutable(const boost::hana::basic_type<T>& /*unused*/)
    {
        return std::is_const<T>::value;
    }

    template <typename T>
    static type get_associated_type(const boost::hana::basic_type<T>& /*unused*/)
    {
        return associated_qubus_type<typename std::remove_cv<T>::type>::get();
    }

    static std::vector<type> get_parameter_types()
    {
        std::vector<type> param_types;

        boost::hana::for_each(boost::hana::make_tuple(boost::hana::type_c<Parameters>...),
                              [&param_types](auto param) {
                                  if (is_immutable(param))
                                  {
                                      param_types.push_back(get_associated_type(param));
                                  }
                              });

        return param_types;
    }

    static type get_result_type()
    {
        std::vector<type> result_types;

        boost::hana::for_each(boost::hana::make_tuple(boost::hana::type_c<Parameters>...),
                              [&result_types](auto param) {
                                  if (!is_immutable(param))
                                  {
                                      result_types.push_back(get_associated_type(param));
                                  }
                              });

        QUBUS_ASSERT(result_types.size() == 1,
                     "Currently, only one mutable parameter is supported.");

        return result_types[0];
    }

    function_declaration generate_thunk()
    {
        std::vector<variable_declaration> params;

        for (const auto& type : get_parameter_types())
        {
            params.push_back(variable_declaration(type));
        }

        std::vector<variable_declaration> results;

        results.push_back(variable_declaration(get_result_type()));

        std::vector<std::unique_ptr<expression>> arguments;

        for (const auto& param : params)
        {
            arguments.push_back(var(param));
        }

        QUBUS_ASSERT(results.size() == 1, "Currently, only one mutable parameter is supported.");

        auto body = foreign_call(computelet_, std::move(arguments), var(results[0]));

        function_declaration thunk("thunk", std::move(params), std::move(results[0]),
                                   std::move(body));

        return thunk;
    }

    foreign_computelet computelet_;
    computelet thunk_;
};

struct foreign_kernel_version_initializer
{
    template <typename Kernel, typename Implementation>
    foreign_kernel_version_initializer(Kernel& kernel, architecture_identifier target,
                                       Implementation implementation)
    {
        kernel.add_version(std::move(target), std::move(implementation));
    }
};

#define QUBUS_ADD_FOREIGN_KERNEL_VERSION(KERNEL, TARGET, IMPLEMENTATION)                           \
    qubus::foreign_kernel_version_initializer                                                      \
        qubus_foreign_kernel_versiob_initializer_##IMPLEMENTATION(KERNEL, TARGET, IMPLEMENTATION); \
    /**/
}

#endif

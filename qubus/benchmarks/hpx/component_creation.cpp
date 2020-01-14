#include <hpx/config.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <hpx/hpx_init.hpp>

#include <nonius/nonius.h++>

#include <iostream>
#include <utility>

class test_component_server : public hpx::components::component_base<test_component_server>
{
};

using test_component_server_type = hpx::components::component<test_component_server>;
HPX_REGISTER_COMPONENT(test_component_server_type, test_component_server);

class test_component : public hpx::components::client_base<test_component, test_component_server>
{
public:
    using base_type = hpx::components::client_base<test_component, test_component_server>;

    test_component() = default;

    test_component(hpx::id_type id) : base_type(std::move(id))
    {
    }

    test_component(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
    {
    }
};

void test()
{
}

void test2()
{
}

HPX_PLAIN_ACTION(test);
HPX_PLAIN_DIRECT_ACTION(test2);

int hpx_main(int argc, char** argv)
{
    nonius::configuration cfg;
    cfg.output_file = "component_creation.html";

    test_component component;

    auto remote_localities = hpx::find_remote_localities();

    if (remote_localities.empty())
    {
        std::cerr << "Error: The benchmark requires at least two localities." << std::endl;

        hpx::finalize();

        return 1;
    }

    nonius::benchmark benchmarks[] = {
        nonius::benchmark("local component creation",
                          [&] { component = hpx::local_new<test_component>(); }),
        nonius::benchmark("remote component creation",
                          [&] { component = hpx::new_<test_component>(remote_localities[0]); }),
        nonius::benchmark("local component creation + wait",
                          [&] { component = hpx::local_new<test_component>(); component.wait(); }),
        nonius::benchmark("remote component creation + wait",
                          [&] { component = hpx::new_<test_component>(remote_localities[0]); component.wait(); }),
        nonius::benchmark("action call",
                          [&] { hpx::sync<test_action>(remote_localities[0]); }),
        nonius::benchmark("direct action call",
                          [&] { hpx::sync<test2_action>(remote_localities[0]); })};

    nonius::go(cfg, std::begin(benchmarks), std::end(benchmarks), nonius::html_reporter());

    nonius::configuration cfg2;

    nonius::go(cfg2, std::begin(benchmarks), std::end(benchmarks), nonius::standard_reporter());

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv);
}
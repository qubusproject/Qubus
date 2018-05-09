#include <qubus/qubus.hpp>

#include <qubus/IR/parsing.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

#include <fstream>

std::string read_code(const std::string& filepath)
{
    std::ifstream fin(filepath);

    auto first = std::istreambuf_iterator<char>(fin);
    auto last = std::istreambuf_iterator<char>();

    std::string code(first, last);

    return code;
}

TEST(lang, extent)
{
    using namespace qubus;

    auto runtime = qubus::get_runtime();

    auto obj_factory = runtime.get_object_factory();

    auto x = obj_factory.create_array(qubus::types::double_{}, {42});
    auto r = obj_factory.create_scalar(qubus::types::integer{});

    auto code = read_code("samples/extent");

    auto mod = qubus::parse_qir(std::move(code));

    runtime.get_module_library().add(std::move(mod)).get();

    qubus::kernel_arguments args;

    args.push_back_arg(x);
    args.push_back_result(r);

    runtime.execute(qubus::symbol_id("extent.test1"), args).get();

    {
        auto r_view = qubus::get_view<host_scalar_view<const util::index_t>>(r).get();

        auto result = r_view.get();

        ASSERT_EQ(result, 42);
    }

    runtime.execute(qubus::symbol_id("extent.test2"), args).get();

    {
        auto r_view = qubus::get_view<host_scalar_view<const util::index_t>>(r).get();

        auto result = r_view.get();

        ASSERT_EQ(result, 15);
    }
}

int hpx_main(int argc, char** argv)
{
    qubus::init(argc, argv);

    auto result = RUN_ALL_TESTS();

    qubus::finalize();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    hpx::resource::partitioner rp(argc, argv, qubus::get_hpx_config(),
                                  hpx::resource::partitioner_mode::mode_allow_oversubscription);

    qubus::setup(rp);

    return hpx::init();
}


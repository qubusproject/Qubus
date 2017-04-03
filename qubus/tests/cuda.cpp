#include <qubus/cuda/core.hpp>

#include <gtest/gtest.h>

TEST(cuda, init)
{
    EXPECT_NO_THROW(qubus::cuda::init());
}

TEST(cuda, device)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    int device_count;
    EXPECT_NO_THROW(device_count = qubus::cuda::get_device_count());

    for (int i = 0; i < device_count; ++i)
    {
        EXPECT_NO_THROW(qubus::cuda::device dev(i));
    }

    EXPECT_THROW(qubus::cuda::device dev(device_count), qubus::cuda::cuda_error);

    std::vector<qubus::cuda::device> devices;

    ASSERT_NO_THROW(devices = qubus::cuda::get_devices());
    EXPECT_EQ(devices.size(), device_count);
}

TEST(cuda, context)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    EXPECT_NO_THROW(qubus::cuda::context ctx);

    for (const auto& device : qubus::cuda::get_devices())
    {
        EXPECT_NO_THROW(qubus::cuda::context ctx(device));
    }
}

TEST(cuda, sync)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    EXPECT_THROW(qubus::cuda::this_context::synchronize(), qubus::cuda::cuda_error);

    if (qubus::cuda::get_device_count() > 0)
    {
        qubus::cuda::device dev(0);

        qubus::cuda::context ctx(dev);

        EXPECT_NO_THROW(qubus::cuda::this_context::synchronize());
    }
}

TEST(cuda, stream)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    EXPECT_THROW(qubus::cuda::stream str, qubus::cuda::cuda_error);

    if (qubus::cuda::get_device_count() > 0)
    {
        qubus::cuda::device dev(0);

        qubus::cuda::context ctx(dev);

        EXPECT_NO_THROW({
            qubus::cuda::stream str;
            str.add_callback([] {});
        });
    }
}

constexpr const char* ptx_code =
    R"(//
       // Generated by NVIDIA NVVM Compiler
       //
       // Compiler Build ID: CL-21554848
       // Cuda compilation tools, release 8.0, V8.0.61
       // Based on LLVM 3.4svn
       //

       .version 5.0
       .target sm_30
       .address_size 64

               // .globl       _Z4testv

       .visible .entry test(

       )
       {



                ret;
       }
       )";

TEST(cuda, module)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    EXPECT_THROW(qubus::cuda::module mod(ptx_code), qubus::cuda::cuda_error);

    if (qubus::cuda::get_device_count() > 0)
    {
        qubus::cuda::device dev(0);

        qubus::cuda::context ctx(dev);

        ASSERT_NO_THROW(qubus::cuda::module mod(ptx_code));

        qubus::cuda::module mod(ptx_code);

        ASSERT_NO_THROW(mod.get_function("test"));
    }
}

TEST(cuda, function)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    if (qubus::cuda::get_device_count() > 0)
    {
        qubus::cuda::device dev(0);

        qubus::cuda::context ctx(dev);

        qubus::cuda::module mod(ptx_code);

        auto test = mod.get_function("test");

        ASSERT_NO_THROW(qubus::cuda::calculate_launch_config_with_max_occupancy(test, 0));

        auto suggested_config = qubus::cuda::calculate_launch_config_with_max_occupancy(test, 0);

        EXPECT_NO_THROW(qubus::cuda::calculate_max_active_blocks_per_multiprocessor(
            test, suggested_config.block_size, 0));

        EXPECT_NO_THROW(qubus::cuda::launch_kernel(test, 1, suggested_config.block_size, 0));

        qubus::cuda::stream nondefault_stream;

        EXPECT_NO_THROW(
            qubus::cuda::launch_kernel(test, 1, suggested_config.block_size, 0, nondefault_stream));

        EXPECT_EQ(test.shared_memory_size(), 0);
        EXPECT_EQ(test.ptx_version(), qubus::cuda::architecture_version(3, 0));

        auto compute_capability = dev.compute_capability();

        EXPECT_EQ(test.binary_version(),
                  qubus::cuda::architecture_version(compute_capability.major_revision,
                                                    compute_capability.minor_revision));
    }
}

TEST(cuda, memory)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    EXPECT_THROW(qubus::cuda::device_malloc(1024), qubus::cuda::cuda_error);

    if (qubus::cuda::get_device_count() > 0)
    {
        qubus::cuda::device dev(0);

        qubus::cuda::context ctx(dev);

        auto ptr = qubus::cuda::device_malloc(1024);

        int value = 42;

        ASSERT_NO_THROW(qubus::cuda::memcpy(ptr, &value, sizeof(int)));

        int other_value;

        ASSERT_NO_THROW(qubus::cuda::memcpy(&other_value, ptr, sizeof(int)));

        EXPECT_EQ(other_value, value);

        EXPECT_NO_THROW(qubus::cuda::device_free(ptr));
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

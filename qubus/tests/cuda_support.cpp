#include <qubus/cuda/core.hpp>
#include <qubus/cuda/support.hpp>

#include <gtest/gtest.h>

#include <hpx/hpx_init.hpp>

TEST(cuda, stream_async_wait)
{
    ASSERT_NO_THROW(qubus::cuda::init());

    if (qubus::cuda::get_device_count() > 0)
    {
        qubus::cuda::device dev(0);

        qubus::cuda::context ctx(dev);

        qubus::cuda::context_guard guard(ctx);

        qubus::cuda::stream s;

        EXPECT_NO_THROW(when_finished(s).get());
    }
}

int hpx_main(int argc, char** argv)
{
    auto result = RUN_ALL_TESTS();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}
#include <qubus/util/one_shot.hpp>

#include <gtest/gtest.h>

TEST(one_shot, empty)
{
    qubus::util::one_shot<int(int)> os;

    EXPECT_FALSE(static_cast<bool>(os));
}

TEST(one_shot, init)
{
    qubus::util::one_shot<int(int)> os([] (int value) { return value; });

    EXPECT_TRUE(static_cast<bool>(os));
}

class move_only_functor
{
public:
    move_only_functor(const move_only_functor&) = delete;
    move_only_functor& operator=(const move_only_functor&) = delete;

    move_only_functor(move_only_functor&&) = default;
    move_only_functor& operator=(move_only_functor&&) = default;

    int operator()(int value) const
    {
        return value;
    }
};

TEST(one_shot, move_only_init)
{
    qubus::util::one_shot<int(int)> os(move_only_functor{});

    EXPECT_TRUE(static_cast<bool>(os));
}

TEST(one_shot, call)
{
    qubus::util::one_shot<int(int)> os([] (int value) { return value; });

    EXPECT_EQ(42, os(42));

    EXPECT_FALSE(static_cast<bool>(os));
}

TEST(one_shot, mutable_call)
{
    qubus::util::one_shot<int()> os([value = 42] () mutable { value = 43; return value; });

    EXPECT_TRUE(static_cast<bool>(os));

    EXPECT_EQ(43, os());

    EXPECT_FALSE(static_cast<bool>(os));
}

TEST(one_shot, void_result_type)
{
    qubus::util::one_shot<void()> os([] { });

    EXPECT_TRUE(static_cast<bool>(os));

    os();

    EXPECT_FALSE(static_cast<bool>(os));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

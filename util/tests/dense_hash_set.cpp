#include <qubus/util/dense_hash_set.hpp>

#include <gtest/gtest.h>

TEST(dense_hash_set, init)
{
    qubus::util::dense_hash_set<int> set;

    EXPECT_EQ(set.load_factor(), 0);
    EXPECT_EQ(set.size(), 0);
    ASSERT_GT(set.max_load_factor(), 0);
    ASSERT_LE(set.load_factor(), set.max_load_factor());
}

TEST(dense_hash_set, insert)
{
    qubus::util::dense_hash_set<int> set;

    auto pos_and_inserted = set.insert(42);

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    EXPECT_TRUE(inserted);
    EXPECT_EQ(*pos, 42);
    EXPECT_EQ(set.size(), 1);
    EXPECT_NE(pos, set.end());
}

TEST(dense_hash_set, emplace)
{
    qubus::util::dense_hash_set<int> set;

    auto pos_and_inserted = set.emplace(42);

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    EXPECT_TRUE(inserted);
    EXPECT_EQ(*pos, 42);
    EXPECT_EQ(set.size(), 1);
    EXPECT_NE(pos, set.end());
}

TEST(dense_hash_set, reinsert)
{
    qubus::util::dense_hash_set<int> set;

    auto pos_and_inserted = set.insert(42);

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    auto pos_and_inserted2 = set.insert(42);

    auto pos2 = pos_and_inserted2.first;
    auto inserted2 = pos_and_inserted2.second;

    EXPECT_FALSE(inserted2);
    EXPECT_EQ(pos, pos2);
    EXPECT_EQ(*pos2, 42);
    EXPECT_EQ(set.size(), 1);
}

TEST(dense_hash_set, find)
{
    qubus::util::dense_hash_set<int> set;

    auto pos_and_inserted = set.insert(42);

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    auto found_pos = set.find(42);

    ASSERT_NE(found_pos, set.end());
    EXPECT_EQ(found_pos, pos);
    EXPECT_EQ(*found_pos, 42);

    auto not_found_pos = set.find(2);

    EXPECT_EQ(not_found_pos, set.end());
}

TEST(dense_hash_set, erase)
{
    qubus::util::dense_hash_set<int> set;

    auto pos_and_inserted = set.insert(42);

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    set.erase(pos);

    auto pos_after_erase = set.find(1);

    EXPECT_EQ(pos_after_erase, set.end());
    EXPECT_EQ(set.size(), 0);
}

TEST(dense_hash_set, erase_key)
{
    qubus::util::dense_hash_set<int> set;

    set.insert(42);

    set.erase(42);

    auto pos = set.find(42);

    EXPECT_EQ(pos, set.end());
    EXPECT_EQ(set.size(), 0);
}

TEST(dense_hash_set, auto_resize)
{
    qubus::util::dense_hash_set<long int> set;

    ASSERT_LT(set.max_load_factor(), 1);
    ASSERT_LE(set.load_factor(), set.max_load_factor());

    for (long int i = 1; i < 21; ++i)
    {
        set.emplace(i);
    }

    EXPECT_LE(set.load_factor(), set.max_load_factor());

    for (long int i = 1; i < 21; ++i)
    {
        auto pos = set.find(i);

        ASSERT_NE(pos, set.end());
        EXPECT_EQ(*pos, i);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}


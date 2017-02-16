#include <qbb/util/dense_hash_map.hpp>

#include <gtest/gtest.h>

TEST(dense_hash_map, init)
{
    util::dense_hash_map<int, int> map(0, 50);

    EXPECT_EQ(map.load_factor(), 0);
    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(map.bucket_count(), 50);
    ASSERT_NE(map.data(), nullptr);
    ASSERT_GT(map.max_load_factor(), 0);
    ASSERT_LE(map.load_factor(), map.max_load_factor());
}

TEST(dense_hash_map, insert)
{
    util::dense_hash_map<int, int> map(0);

    auto pos_and_inserted = map.insert(std::make_pair(1, 42));

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    EXPECT_TRUE(inserted);
    EXPECT_EQ(pos->first, 1);
    EXPECT_EQ(pos->second, 42);
    EXPECT_EQ(map.size(), 1);
    EXPECT_NE(pos, map.invalid_iterator());
}

TEST(dense_hash_map, emplace)
{
    util::dense_hash_map<int, int> map(0);

    auto pos_and_inserted = map.emplace(1, 42);

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    EXPECT_TRUE(inserted);
    EXPECT_EQ(pos->first, 1);
    EXPECT_EQ(pos->second, 42);
    EXPECT_EQ(map.size(), 1);
    EXPECT_NE(pos, map.invalid_iterator());
}

TEST(dense_hash_map, reinsert)
{
    util::dense_hash_map<int, int> map(0);

    auto pos_and_inserted = map.insert(std::make_pair(1, 42));

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    auto pos_and_inserted2 = map.insert(std::make_pair(1, 43));

    auto pos2 = pos_and_inserted2.first;
    auto inserted2 = pos_and_inserted2.second;

    EXPECT_FALSE(inserted2);
    EXPECT_EQ(pos, pos2);
    EXPECT_EQ(pos2->first, 1);
    EXPECT_EQ(pos2->second, 42);
    EXPECT_EQ(map.size(), 1);
}

TEST(dense_hash_map, find)
{
    util::dense_hash_map<int, int> map(0);

    auto pos_and_inserted = map.insert(std::make_pair(1, 42));

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    auto found_pos = map.find(1);

    ASSERT_NE(found_pos, map.invalid_iterator());
    EXPECT_EQ(found_pos, pos);
    EXPECT_EQ(found_pos->first, 1);
    EXPECT_EQ(found_pos->second, 42);

    auto not_found_pos = map.find(2);

    EXPECT_EQ(not_found_pos, map.invalid_iterator());
}

TEST(dense_hash_map, at)
{
    util::dense_hash_map<int, int> map(0);

    auto pos_and_inserted = map.insert(std::make_pair(1, 42));

    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    int value;

    EXPECT_NO_THROW({ value = map.at(1); });

    EXPECT_EQ(value, 42);

    EXPECT_THROW(map.at(2), std::out_of_range);
}

TEST(dense_hash_map, erase)
{
    util::dense_hash_map<int, int> map(0);

    auto pos_and_inserted = map.insert(std::make_pair(1, 42));

    auto pos = pos_and_inserted.first;
    auto inserted = pos_and_inserted.second;

    ASSERT_TRUE(inserted);

    map.erase(pos);

    auto pos_after_erase = map.find(1);

    EXPECT_EQ(pos_after_erase, map.invalid_iterator());
    EXPECT_EQ(map.size(), 0);
}

TEST(dense_hash_map, erase_key)
{
    util::dense_hash_map<int, int> map(0);

    map.insert(std::make_pair(1, 42));

    map.erase(1);

    auto pos = map.find(1);

    EXPECT_EQ(pos, map.invalid_iterator());
    EXPECT_EQ(map.size(), 0);
}

TEST(dense_hash_map, auto_resize)
{
    util::dense_hash_map<long int, long int> map(0, 20);

    ASSERT_EQ(map.bucket_count(), 20);
    ASSERT_LT(map.max_load_factor(), 1);
    ASSERT_LE(map.load_factor(), map.max_load_factor());

    for (long int i = 1; i < 21; ++i)
    {
        map.emplace(i, i);
    }

    EXPECT_GT(map.bucket_count(), 20);
    EXPECT_LE(map.load_factor(), map.max_load_factor());

    for (long int i = 1; i < 21; ++i)
    {
        auto pos = map.find(i);

        ASSERT_NE(pos, map.invalid_iterator());
        EXPECT_EQ(pos->first, i);
        EXPECT_EQ(pos->second, i);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

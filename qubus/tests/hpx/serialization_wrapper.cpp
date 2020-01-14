#include <hpx/hpx_init.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/serialization.hpp>

#include <gtest/gtest.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <variant>

class allocator
{
public:
    void* allocate(std::size_t size)
    {
        allocated_memory_size_ += size;

        return std::malloc(size);
    }

    std::size_t allocated_memory_size() const
    {
        return allocated_memory_size_;
    }

private:
    std::size_t allocated_memory_size_ = 0;
};

struct data
{
public:
    static constexpr long int test_size = 10 * 1024 * 1024;

    data() : metadata(42), ptr(reinterpret_cast<char*>(std::malloc(test_size))), size(test_size)
    {
        for (long int i = 0; i < test_size; ++i)
        {
            ptr[i] = static_cast<char>(i % 128);
        }
    }

    data(int metadata, char* ptr, std::size_t size) : metadata(metadata), ptr(ptr), size(size)
    {
    }

    ~data()
    {
        std::free(ptr);
    }

    data(const data&) = delete;
    data& operator=(const data&) = delete;

    data(data&& other) : metadata(other.metadata), ptr(other.ptr), size(other.size)
    {
        other.ptr = nullptr;
    }

    data& operator=(data&& other)
    {
        this->metadata = other.metadata;
        this->ptr = other.ptr;
        this->size = other.size;

        other.ptr = nullptr;

        return *this;
    }

    int metadata;
    char* ptr;
    std::size_t size;
};

namespace
{
class zero_copy_unwrapper
{
public:
    using result_type = data;

    explicit zero_copy_unwrapper(allocator* alloc_) : alloc_(alloc_)
    {
    }

    data operator()(data&& value) const
    {
        return std::move(value);
    }

    data operator()(const data* value) const
    {
        assert(alloc_ != nullptr);

        data copy(value->metadata, reinterpret_cast<char*>(alloc_->allocate(value->size)),
                  value->size);

        std::memcpy(copy.ptr, value->ptr, value->size);

        return copy;
    }

private:
    allocator* alloc_;
};

struct conditional_data_dereferencer
{
    using result_type = const data&;

    result_type operator()(const data& value) const
    {
        return value;
    }

    result_type operator()(const data* value) const
    {
        assert(value != nullptr);

        return *value;
    }
};
} // namespace

class zero_copy_wrapper
{
public:
    zero_copy_wrapper() = default;

    explicit zero_copy_wrapper(const data& data_, hpx::id_type source_locality_,
                               std::uintptr_t allocator_addr_)
    : holder_(&data_), source_locality_(source_locality_), allocator_addr_(allocator_addr_)
    {
    }

    zero_copy_wrapper(const zero_copy_wrapper&) = delete;
    zero_copy_wrapper& operator=(const zero_copy_wrapper&) = delete;

    zero_copy_wrapper(zero_copy_wrapper&&) = default;
    zero_copy_wrapper& operator=(zero_copy_wrapper&&) = default;

    data unwrap()
    {
        assert(hpx::find_here() == source_locality_);

        allocator* alloc;

        std::memcpy(&alloc, &allocator_addr_, sizeof(allocator*));

        return std::visit(zero_copy_unwrapper(alloc), std::move(holder_));
    }

    template <typename Archive>
    void load(Archive& ar, unsigned int)
    {
        int metadata = 0;
        ar >> metadata;

        std::size_t size = 0;
        ar >> size;

        ar >> allocator_addr_;

        ar >> source_locality_;

        assert(hpx::find_here() == source_locality_);

        allocator* alloc;

        std::memcpy(&alloc, &allocator_addr_, sizeof(allocator*));

        auto& transfered_data =
            holder_.emplace<data>(metadata, reinterpret_cast<char*>(alloc->allocate(size)), size);

        ar >> hpx::serialization::make_array(transfered_data.ptr, transfered_data.size);
    }

    template <typename Archive>
    void save(Archive& ar, unsigned int) const
    {
        auto& ref_ = std::visit(conditional_data_dereferencer(), holder_);

        ar << ref_.metadata;

        ar << ref_.size;

        ar << allocator_addr_;

        ar << source_locality_;

        ar << hpx::serialization::make_array(ref_.ptr, ref_.size);
    }

    HPX_SERIALIZATION_SPLIT_MEMBER();

private:
    std::variant<const data*, data> holder_;
    hpx::id_type source_locality_;
    std::uintptr_t allocator_addr_ = 0;
};

data test_data;

zero_copy_wrapper transfer_impl(hpx::id_type source_locality, std::uintptr_t allocator_addr)
{
    return zero_copy_wrapper(test_data, source_locality, allocator_addr);
}

HPX_PLAIN_ACTION(transfer_impl);

data transfer(hpx::id_type remote_locality, allocator& alloc)
{
    void* ptr = &alloc;
    std::uintptr_t allocator_addr;

    std::memcpy(&allocator_addr, &ptr, sizeof(std::uintptr_t));

    auto wrapper =
        hpx::sync<transfer_impl_action>(remote_locality, hpx::find_here(), allocator_addr);

    return wrapper.unwrap();
}

TEST(hpx, serialization_wrapper_remote)
{
    auto remote_localities = hpx::find_remote_localities();

    assert(!remote_localities.empty());

    allocator alloc;

    auto data = transfer(remote_localities[0], alloc);

    ASSERT_EQ(alloc.allocated_memory_size(), data::test_size);
    ASSERT_EQ(data.size, data::test_size);

    for (long int i = 0; i < data.size; ++i)
    {
        ASSERT_EQ(data.ptr[i], i % 128);
    }
}

TEST(hpx, serialization_wrapper_local)
{
    allocator alloc;

    auto data = transfer(hpx::find_here(), alloc);

    ASSERT_EQ(alloc.allocated_memory_size(), data::test_size);
    ASSERT_EQ(data.size, data::test_size);

    for (long int i = 0; i < data.size; ++i)
    {
        ASSERT_EQ(data.ptr[i], i % 128);
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

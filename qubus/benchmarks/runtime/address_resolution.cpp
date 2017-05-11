#include <hpx/config.hpp>

#include <qubus/allocator.hpp>
#include <qubus/memory_block.hpp>
#include <qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <nonius/nonius.h++>

#include <memory>

class mock_memory_block : public qubus::memory_block
{
public:
    ~mock_memory_block() override = default;

    std::size_t size() const override
    {
        return 0;
    }

    void* ptr() const override
    {
        return nullptr;
    }
};

class mock_allocator : public qubus::allocator
{
public:
    ~mock_allocator() override = default;

    std::unique_ptr<qubus::memory_block> allocate(std::size_t size, std::size_t alignment) override
    {
        return std::make_unique<mock_memory_block>();
    }

    void deallocate(qubus::memory_block& mem_block) override
    {
    }
};

int hpx_main(int argc, char** argv)
{
    qubus::init(argc, argv);

    auto obj_factory = qubus::get_runtime().get_object_factory();

    std::vector<qubus::object> filler_objects;

    for (int i = 0; i < 500; ++i)
    {
        filler_objects.push_back(obj_factory.create_scalar(qubus::types::double_{}));
    }

    qubus::object obj = obj_factory.create_scalar(qubus::types::double_{});

    for (int i = 0; i < 500; ++i)
    {
        filler_objects.push_back(obj_factory.create_scalar(qubus::types::double_{}));
    }

    qubus::object obj2 = obj_factory.create_scalar(qubus::types::double_{});

    for (int i = 0; i < 500; ++i)
    {
        filler_objects.push_back(obj_factory.create_scalar(qubus::types::double_{}));
    }

    qubus::host_address_space addr_space(std::make_unique<mock_allocator>());

    addr_space.allocate_object_page(obj, sizeof(double), sizeof(void*));

    addr_space.on_page_fault(
        [](const qubus::object& faulted_obj, qubus::address_space::page_fault_context ctx)
            -> hpx::future<qubus::address_space::address_entry> {

            auto page = ctx.allocate_page(faulted_obj, 0, sizeof(void*));

            return hpx::make_ready_future(std::move(page));
        });

    nonius::configuration cfg;
    cfg.output_file = "address_resolution.html";

    nonius::benchmark benchmarks[] = {
        nonius::benchmark("page hit", [&] { addr_space.resolve_object(obj); }),
        nonius::benchmark("page fault", [&] { addr_space.resolve_object(obj2); }),
        nonius::benchmark("try page hit", [&] { addr_space.try_resolve_object(obj); }),
        nonius::benchmark("try page fault", [&] { addr_space.try_resolve_object(obj2); })};

    nonius::go(cfg, std::begin(benchmarks), std::end(benchmarks), nonius::html_reporter());

    nonius::configuration cfg2;

    nonius::go(cfg2, std::begin(benchmarks), std::end(benchmarks), nonius::standard_reporter());

    qubus::finalize();

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv, qubus::get_hpx_config());
}

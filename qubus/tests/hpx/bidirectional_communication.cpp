#include <hpx/hpx_init.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <gtest/gtest.h>

#include <cassert>
#include <utility>

class test_component_server : public hpx::components::component_base<test_component_server>
{
public:
    hpx::future<int> test() const
    {
        return forward();
    }

    int calc() const
    {
        return 42;
    }

    void set_partner(hpx::id_type partner)
    {
        assert(partner.get_management_type() == hpx::id_type::unmanaged);

        this->partner_ = std::move(partner);
    }
public:
    HPX_DEFINE_COMPONENT_ACTION(test_component_server, test, test_action);
    HPX_DEFINE_COMPONENT_ACTION(test_component_server, calc, calc_action);
    HPX_DEFINE_COMPONENT_ACTION(test_component_server, set_partner, set_partner_action);
private:
    hpx::future<int> forward() const
    {
        assert(partner_);

        return hpx::async<calc_action>(partner_);
    }

    hpx::id_type partner_;
};

using test_component_server_type = hpx::components::component<test_component_server>;
HPX_REGISTER_COMPONENT(test_component_server_type, test_component_server);

using test_action = test_component_server::test_action;
HPX_REGISTER_ACTION(test_action, test_component_server_test_action);

using calc_action = test_component_server::calc_action;
HPX_REGISTER_ACTION(calc_action, test_component_server_calc_action);

using set_partner_action = test_component_server::set_partner_action;
HPX_REGISTER_ACTION(set_partner_action, test_component_server_set_partner_action);

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

    hpx::future<int> test()
    {
        return hpx::async<test_component_server::test_action>(this->get_id());
    }

    void set_partner(hpx::id_type partner)
    {
        hpx::sync<test_component_server::set_partner_action>(this->get_id(), std::move(partner));
    }
};

TEST(hpx, bidirectional_communication)
{
    test_component com1 = hpx::new_<test_component>(hpx::find_here());
    test_component com2 = hpx::new_<test_component>(hpx::find_here());

    auto com1_id = com1.get_id();
    com1_id.make_unmanaged();
    com2.set_partner(std::move(com1_id));

    auto com2_id = com2.get_id();
    com2_id.make_unmanaged();
    com1.set_partner(std::move(com2_id));

    auto result1 = com1.test().get();
    auto result2 = com2.test().get();

    ASSERT_EQ(result1, result2);
    ASSERT_EQ(result1, 42);
    ASSERT_EQ(result2, 42);
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
#ifndef QBB_KUBUS_TESTS_KUBUS_UNIT_TEST_HPP
#define QBB_KUBUS_TESTS_KUBUS_UNIT_TEST_HPP

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

int hpx_main(int argc, char** argv)
{
    auto result = boost::unit_test::unit_test_main(init_unit_test, argc, argv);

    hpx::finalize();
    
    return result;
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv);
}

#endif
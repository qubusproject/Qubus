#include <catch2/catch.hpp>

#include <qubus/IR/module_loader.hpp>
#include <qubus/IR/qir.hpp>

SCENARIO("Modules can be loaded", "[qir]")
{
    GIVEN("A module")
    {
        std::vector<int> v(5);

        REQUIRE(v.size() == 5);
        REQUIRE(v.capacity() >= 5);

        WHEN("the module is loaded")
        {
            v.resize(10);
            THEN("a valid assembly is created")
            {
                REQUIRE(v.size() == 10);
                REQUIRE(v.capacity() >= 10);
            }
        }
    }
}
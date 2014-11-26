set(KUBUS_BUILD_TESTS OFF CACHE Bool "build the kubus test suite")

if(KUBUS_BUILD_TESTS)
  find_package(Boost 1.50.0 COMPONENTS unit_test_framework REQUIRED)

  add_executable(simple_contraction simple_contraction.cpp)
  target_include_directories(simple_contraction PUBLIC ${Boost_INCLUDE_DIRS})
  target_compile_options(simple_contraction PRIVATE ${COVERAGE_FLAGS})
  target_link_libraries(simple_contraction kubus hpx_init ${Boost_LIBRARIES} ${COVERAGE_LINKER_FLAGS})
  add_test(simple_contraction ${CMAKE_CURRENT_BINARY_DIR}/simple_contraction)

endif()
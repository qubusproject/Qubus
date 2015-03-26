set(KUBUS_BUILD_TESTS OFF CACHE Bool "build the kubus test suite")

if(KUBUS_BUILD_TESTS)
  find_package(GTest REQUIRED)

  add_executable(simple_contraction simple_contraction.cpp)
  target_include_directories(simple_contraction PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(simple_contraction kubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(simple_contraction ${CMAKE_CURRENT_BINARY_DIR}/simple_contraction)

endif()
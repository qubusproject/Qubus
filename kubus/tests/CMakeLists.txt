set(KUBUS_BUILD_TESTS OFF CACHE Bool "build the kubus test suite")

if(KUBUS_BUILD_TESTS)
  find_package(GTest REQUIRED)

  add_executable(contractions contractions.cpp)
  target_include_directories(contractions PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(contractions kubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(contractions ${CMAKE_CURRENT_BINARY_DIR}/contractions)

endif()
option(QUBUS_BUILD_TESTS "build the kubus test suite" OFF)

if(QUBUS_BUILD_TESTS)
  find_package(GTest REQUIRED)

  add_executable(basic_expressions basic_expressions.cpp)
  target_include_directories(basic_expressions PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(basic_expressions kubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(basic_expressions ${CMAKE_CURRENT_BINARY_DIR}/basic_expressions)

  add_executable(contractions contractions.cpp)
  target_include_directories(contractions PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(contractions kubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(contractions ${CMAKE_CURRENT_BINARY_DIR}/contractions)

  add_executable(multi_indices multi_indices.cpp)
  target_include_directories(multi_indices PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(multi_indices kubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(multi_indices ${CMAKE_CURRENT_BINARY_DIR}/multi_indices)
endif()
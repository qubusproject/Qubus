option(QUBUS_BUILD_TESTS "build the Qubus test suite" OFF)

if(QUBUS_BUILD_TESTS)
  find_package(GTest REQUIRED)

  add_executable(basic_expressions basic_expressions.cpp)
  target_include_directories(basic_expressions PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(basic_expressions qubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(basic_expressions ${CMAKE_CURRENT_BINARY_DIR}/basic_expressions)

  add_executable(contractions contractions.cpp)
  target_include_directories(contractions PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(contractions qubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(contractions ${CMAKE_CURRENT_BINARY_DIR}/contractions)

  add_executable(multi_indices multi_indices.cpp)
  target_include_directories(multi_indices PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(multi_indices qubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(multi_indices ${CMAKE_CURRENT_BINARY_DIR}/multi_indices)

  add_executable(qm_patterns qm_patterns.cpp)
  target_include_directories(qm_patterns PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(qm_patterns qubus hpx_init ${GTEST_BOTH_LIBRARIES})
  add_test(qm_patterns ${CMAKE_CURRENT_BINARY_DIR}/qm_patterns)

  add_executable(sparse_support sparse_support.cpp)
  target_include_directories(sparse_support PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(sparse_support qubus hpx_init ${GTEST_BOTH_LIBRARIES})
  target_compile_options(sparse_support PRIVATE)
  add_test(sparse_support ${CMAKE_CURRENT_BINARY_DIR}/sparse_support)
endif()
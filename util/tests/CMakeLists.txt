option(QUBUS_UTIL_BUILD_TESTS "build the Qubus util test suite" OFF)

function(util_add_test name src_file)
  add_executable(${name} ${src_file})
  target_include_directories(${name} PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(${name} qubus_util ${GTEST_BOTH_LIBRARIES})
  add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name})
endfunction()

function(util_add_simple_test name)
  util_add_test(${name} ${name}.cpp)
endfunction()

if(QUBUS_UTIL_BUILD_TESTS)
  find_package(GTest REQUIRED)

  util_add_simple_test(one_shot)
  util_add_simple_test(dense_hash_map)
endif()
option(QUBUS_BUILD_TESTS "build the Qubus test suite" OFF)

function(qubus_add_test name src_file)
  add_executable(${name} ${src_file})
  target_include_directories(${name} PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(${name} qubus ${GTEST_BOTH_LIBRARIES} hpx hpx_init)
  add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name})
endfunction()

function(qubus_add_simple_test name)
  qubus_add_test(${name} ${name}.cpp)
endfunction()

function(qubus_qtl_add_test name src_file)
  add_executable(${name} ${src_file})
  target_include_directories(${name} PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(${name} qubus_qtl qubus ${GTEST_BOTH_LIBRARIES} hpx hpx_init)
  add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name})
endfunction()

function(qubus_qtl_add_simple_test name)
  qubus_qtl_add_test(${name} ${name}.cpp)
endfunction()

if(QUBUS_BUILD_TESTS)
  find_package(GTest REQUIRED)
  find_package(Catch2 REQUIRED)

  add_library(catch2_runner OBJECT catch2_runner.cpp)
  include(Catch)

  qubus_qtl_add_simple_test(basic_expressions)
  qubus_qtl_add_simple_test(contractions)
  qubus_qtl_add_simple_test(multi_indices)
  qubus_qtl_add_simple_test(qm_patterns)
  #qubus_qtl_add_simple_test(sparse_support)
  qubus_qtl_add_simple_test(index_semantic)
  qubus_qtl_add_simple_test(parametrized_kernel)
  qubus_qtl_add_simple_test(kronecker_delta)
  qubus_add_simple_test(alias_analysis)
  qubus_add_simple_test(axiom_analysis)
  qubus_add_simple_test(task_invariants_analysis)
  qubus_add_simple_test(value_set_analysis)
  qubus_add_simple_test(value_range_analysis)
  qubus_add_simple_test(static_schedule_analysis)
  qubus_qtl_add_simple_test(slicing)
  qubus_add_simple_test(symbolic_regression)
  qubus_add_simple_test(variable_access_analysis)
  #qubus_qtl_add_simple_test(foreign_kernels)
  qubus_qtl_add_simple_test(scalar_support)
  qubus_add_simple_test(symbol_id)
  qubus_add_simple_test(module)
  qubus_add_simple_test(lang)

  add_executable(parsing parsing.cpp)
  target_include_directories(parsing PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(parsing PRIVATE qubus_ir ${GTEST_BOTH_LIBRARIES})
  add_test(parsing ${CMAKE_CURRENT_BINARY_DIR}/parsing)

  add_custom_command(TARGET parsing POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/samples $<TARGET_FILE_DIR:parsing>/samples)

  add_executable(pretty_print pretty_print.cpp)
  target_include_directories(pretty_print PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(pretty_print PRIVATE qubus_ir ${GTEST_BOTH_LIBRARIES})
  add_test(pretty_print ${CMAKE_CURRENT_BINARY_DIR}/pretty_print)

  if (QUBUS_HAS_FUZZING_SUPPORT)
    add_executable(parsing_fuzz parsing_fuzz.cpp)
    target_link_libraries(parsing_fuzz PRIVATE qubus_ir)
    target_compile_options(parsing_fuzz PRIVATE ${QUBUS_FUZZING_FLAGS})
    target_link_libraries(parsing_fuzz PRIVATE ${QUBUS_FUZZING_LINK_FLAGS})
    add_test(parsing_fuzz ${CMAKE_CURRENT_BINARY_DIR}/parsing)

    add_custom_command(TARGET parsing_fuzz POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/samples $<TARGET_FILE_DIR:parsing_fuzz>/corpus)
  endif()

  if (TARGET qubus_cuda)
    add_executable(cuda cuda.cpp)
    target_include_directories(cuda PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(cuda qubus_cuda ${GTEST_BOTH_LIBRARIES})
    add_test(cuda ${CMAKE_CURRENT_BINARY_DIR}/cuda)

    add_executable(cuda_support cuda_support.cpp)
    target_include_directories(cuda_support PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(cuda_support qubus_cuda ${GTEST_BOTH_LIBRARIES} hpx hpx_init)
    add_test(cuda_support ${CMAKE_CURRENT_BINARY_DIR}/cuda)
  endif()

  add_executable(runtime_library runtime_library.cpp)
  target_include_directories(runtime_library PUBLIC ${GTEST_INCLUDE_DIRS})
  target_link_libraries(runtime_library qubus_jit ${GTEST_BOTH_LIBRARIES})
  add_test(runtime_library ${CMAKE_CURRENT_BINARY_DIR}/runtime_library)

  add_executable(qir $<TARGET_OBJECTS:catch2_runner> module_loader.cpp)
  target_link_libraries(qir PRIVATE Catch2::Catch2 qubus_ir)
  catch_discover_tests(qir)
endif()

add_subdirectory(hpx)
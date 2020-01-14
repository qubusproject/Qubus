if(QUBUS_BUILD_TESTS)
    add_executable(bidirectional_communication bidirectional_communication.cpp)
    target_include_directories(bidirectional_communication PUBLIC ${HPX_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(bidirectional_communication PRIVATE ${GTEST_BOTH_LIBRARIES} hpx hpx_init)
    add_test(NAME bidirectional_communication COMMAND bidirectional_communication)

    if (HPX_WITH_PARCELPORT_MPI)
        find_package(MPI COMPONENTS CXX REQUIRED)

        add_executable(serialization_wrapper serialization_wrapper.cpp)
        target_include_directories(serialization_wrapper PUBLIC ${HPX_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
        target_link_libraries(serialization_wrapper PRIVATE ${GTEST_BOTH_LIBRARIES} hpx hpx_init)
        add_test(NAME serialization_wrapper COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2
                                                        ${MPIEXEC_PREFLAGS} serialization_wrapper ${MPIEXEC_POSTFLAGS})
    endif()
endif()

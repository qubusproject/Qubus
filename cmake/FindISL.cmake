include(FindPackageHandleStandardArgs)

find_path(ISL_INCLUDE_DIR isl/version.h)

find_library(ISL_LIBRARY NAMES isl)

if(NOT TARGET isl::isl)
    add_library(isl::isl UNKNOWN IMPORTED)

    set_target_properties(isl::isl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${ISL_INCLUDE_DIR}
                                              IMPORTED_LOCATION ${ISL_LIBRARY}
                                              INTERFACE_LINK_LIBRARIES ${ISL_LIBRARY})
endif()

set(ISL_LIBRARIES isl::isl)
set(ISL_INCLUDE_DIRS ${ISL_INCLUDE_DIR})

find_package_handle_standard_args(ISL DEFAULT_MSG ISL_LIBRARY ISL_INCLUDE_DIR) 

mark_as_advanced(ISL_LIBRARY ISL_INCLUDE_DIR)
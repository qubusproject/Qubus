include(CMakeParseArguments)

function(qubus_target_header_files)
    set(options )
    set(oneValueArgs TARGET ROOT)
    set(multiValueArgs FILES)
    cmake_parse_arguments(qubus_target_header_files "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(file IN LISTS qubus_target_header_files_FILES)
        list(APPEND header_files ${qubus_target_header_files_ROOT}/${file})
    endforeach()

    target_sources(${qubus_target_header_files_TARGET} PRIVATE ${header_files})
endfunction()
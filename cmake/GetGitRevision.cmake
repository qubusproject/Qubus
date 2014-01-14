macro(GetGitRevision)

find_package(Git)

execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
                OUTPUT_VARIABLE KSMF_GIT_REVISION
                RESULT_VARIABLE KSMF_GET_GIT_REVISION_RESULT
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT ${KSMF_GET_GIT_REVISION_RESULT} EQUAL 0)
   set(KSMF_GIT_REVISION "unknown")
endif(NOT ${KSMF_GET_GIT_REVISION_RESULT} EQUAL 0) 

message(STATUS "current Git revision is: ${KSMF_GIT_REVISION}")

endmacro(GetGitRevision)

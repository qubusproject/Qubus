macro(qubus_set_output_name target name)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME ${name}
                                               OUTPUT_NAME_DEBUG ${name}
            OUTPUT_NAME_RELEASE ${name}
            OUTPUT_NAME_RELWITHDEBINFO ${name}
            OUTPUT_NAME_MINSIZEREL ${name}
            OUTPUT_NAME_DEBUG ${name})
endmacro()
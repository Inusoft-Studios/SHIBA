include(CheckIPOSupported)
check_ipo_supported(RESULT _shiba_ipo_ok OUTPUT _shiba_ipi_msg LANGUAGES CXX)

function(shiba_enable_ipo target)
    if (_shiba_ipo_ok)
        set_target_properties(${target} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION_RELEASE        ON
                INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
    endif ()
endfunction()
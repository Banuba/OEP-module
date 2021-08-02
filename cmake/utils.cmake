#
# Helper functions and utils for CMake
#

include(CMakeParseArguments)

#
# Macro for setting default variable values, if the other value one haven't been
# passed from outside. We use it primarily for variables passed from
# build dodo build system.
#

macro(set_variable_if_undefined VARIABLE DEFAULT)
    if(NOT (DEFINED ${VARIABLE}))
        set(${VARIABLE} ${DEFAULT})
        message("${VARIABLE}: ${${VARIABLE}} (default value)")
    else()
        message("${VARIABLE}: ${${VARIABLE}} (defined externally)")
    endif()
endmacro()
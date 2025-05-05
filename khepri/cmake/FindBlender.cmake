# Find Blender
#
# This module attempts to find the Blender software
# FindBlender provides the following variables:
#
# Blender_FOUND      - has Blender been found
# Blender_EXECUTABLE - Path to Blender executable
# Blender_VERSION    - Version of Blender

SET(_BLENDER_HINTS
    $ENV{BLENDER_HOME}
)

SET(_BLENDER_ROOTS
    "$ENV{programfiles}/Blender Foundation"
)

SET(_BLENDER_PATHS)
foreach(ROOT ${_BLENDER_ROOTS})
    if(EXISTS ${ROOT})
        file(GLOB PATHS LIST_DIRECTORIES TRUE "${ROOT}/*")
        foreach(PATH ${PATHS})
            list(APPEND _BLENDER_PATHS "${PATH}")
        endforeach()
    endif()
endforeach()

find_program(Blender_EXECUTABLE
    NAMES blender
    HINTS ${_BLENDER_HINTS}
    PATHS ${_BLENDER_PATHS}
)

if (Blender_EXECUTABLE)
    # Extract version
    execute_process(
        COMMAND "${Blender_EXECUTABLE}" --version
        RESULT_VARIABLE res
        OUTPUT_VARIABLE var
        ERROR_VARIABLE var
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if (res EQUAL 0 AND var MATCHES "Blender \([0-9]+\\.[0-9]+\)")
        set(Blender_VERSION "${CMAKE_MATCH_1}")
    else()
        message(WARNING "Error getting blender version")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Blender
    FOUND_VAR Blender_FOUND
    REQUIRED_VARS
        Blender_EXECUTABLE
    VERSION_VAR Blender_VERSION
)

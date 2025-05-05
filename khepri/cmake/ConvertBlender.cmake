#
# Blender conversion functions
#
# This module provides methods to convert of Blender asset files to Khepri-specific
# asset formats.
#
find_package(Blender REQUIRED)

set(BLENDCONV_EXPORT_PY "${CMAKE_CURRENT_LIST_DIR}/ConvertBlender/ExportCollada.py")

#
# Converts a Blender .blend file to a Khepri Model Format (KMF) file.
#
# Usage:
#
#   blender_to_kmf(
#     BLEND <BLEND_PATH>
#     KMF   <KMF_PATH>
#     LOG   <LOG_PATH>
#   )
#
# This will create a set of custom_commands to convert the .blend file at <BLEND_PATH>
# to a KMF file at <KMF_PATH>. A conversion log will be written to <LOG_PATH>.
# Add a CMake dependency from a target to <KMF_PATH> to trigger the conversion.
#
# The generated commands require Blender and Khepri's `dae2kmf` tool for the conversion.
#
function(blender_to_kmf)
    cmake_parse_arguments(BLENDCONV "" "BLEND;KMF;LOG" "" ${ARGN})
    if("${BLENDCONV_BLEND}" STREQUAL "")
      message(FATAL_ERROR "Missing BLEND option")
    elseif("${BLENDCONV_KMF}" STREQUAL "")
      message(FATAL_ERROR "Missing KMF option")
    elseif("${BLENDCONV_LOG}" STREQUAL "")
      message(FATAL_ERROR "Missing LOG option")
    else()
        set(BLENDCONV_DAE "${BLENDCONV_KMF}.dae")

        # Use Blender to export .blend file to a Collada (dae) file.
        add_custom_command(
            OUTPUT "${BLENDCONV_DAE}"
            COMMAND ${Blender_EXECUTABLE} --factory-startup --background ${BLENDCONV_BLEND} -P ${BLENDCONV_EXPORT_PY} -- ${BLENDCONV_DAE} > ${BLENDCONV_LOG}
            VERBATIM
            MAIN_DEPENDENCY "${BLENDCONV_BLEND}"
            DEPENDS "${BLENDCONV_EXPORT_PY}"
            BYPRODUCTS "${MODEL_LOG}"
        )

        # Use our dae2kmf tool to convert the Collada (dae) file to a KMF file.
        add_custom_command(
            OUTPUT "${BLENDCONV_KMF}"
            COMMAND $<TARGET_FILE:dae2kmf> -i ${BLENDCONV_DAE} -o ${BLENDCONV_KMF}
            VERBATIM
            MAIN_DEPENDENCY "${BLENDCONV_DAE}"
            DEPENDS dae2kmf
        )
    endif()
endfunction()

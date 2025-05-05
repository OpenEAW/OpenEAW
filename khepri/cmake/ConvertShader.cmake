#
# Shader conversion functions
#
# This module provides methods to convert GLSL shader files to a Khepri-specific
# shader format.
#

#
# Compiles a GLSL shader file to a Khepri Shader Format (KSF) file.
#
# Usage:
#
#   shader_to_ksf(
#     SHADER <SHADER_PATH>
#     KSF    <KSF_PATH>
#   )
#
# This will create a set of custom_commands to convert the shader file at <SHADER_PATH>
# to a KSF file at <KSF_PATH>.
# Add a CMake dependency from a target to <KSF_PATH> to trigger the conversion.
#
# The generated commands require Khepri's `glsl2ksf` tool for the conversion.
#
function(shader_to_ksf)
    cmake_parse_arguments(SHADERCONV "" "SHADER;KSF" "" ${ARGN})
    if("${SHADERCONV_SHADER}" STREQUAL "")
      message(FATAL_ERROR "Missing SHADER option")
    elseif("${SHADERCONV_KSF}" STREQUAL "")
      message(FATAL_ERROR "Missing KSF option")
    else()
        # Use our glsl2ksf tool to convert the shader file to a KSF file.
        add_custom_command(
            OUTPUT "${SHADERCONV_KSF}"
            COMMAND $<TARGET_FILE:glsl2ksf> -i ${SHADERCONV_SHADER} -o ${SHADERCONV_KSF}
            VERBATIM
            MAIN_DEPENDENCY "${SHADERCONV_SHADER}"
            DEPENDS glsl2ksf
        )
    endif()
endfunction()

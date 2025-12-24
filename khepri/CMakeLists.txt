cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(Khepri CXX)

# External libraries
find_package(diligent-core REQUIRED)
find_package(fmt REQUIRED)
find_package(freetype REQUIRED)
find_package(glfw3 REQUIRED)
find_package(gsl-lite REQUIRED)

add_library(${PROJECT_NAME}
    src/adapters/window_input.cpp
    src/application/console_logger.cpp
    src/application/current_directory.cpp
    src/application/exceptions.cpp
    src/application/window.cpp
    src/font/io/font_face.cpp
    src/font/font_cache.cpp
    src/font/font_face_state.cpp
    src/font/font_face.cpp
    src/font/font.cpp
    src/game/rts_camera.cpp
    src/io/container_stream.cpp
    src/io/file.cpp
    src/io/stream.cpp
    src/log/log.cpp
    src/math/interpolator.cpp
    src/math/polynomial.cpp
    src/math/spline.cpp
    src/physics/collision_mesh.cpp
    src/renderer/io/kmf.cpp
    src/renderer/io/texture.cpp
    src/renderer/io/texture_dds.cpp
    src/renderer/io/texture_tga.cpp
    src/renderer/io/shader.cpp
    src/renderer/camera.cpp
    src/renderer/model.cpp
    src/renderer/texture_desc.cpp
    src/renderer/diligent/native_window.cpp
    src/renderer/diligent/renderer.cpp
    src/scene/scene_object.cpp
    src/utility/crc.cpp
    src/utility/platform.cpp
    src/utility/string.cpp
    src/version_info.cpp
)

set_property(
  SOURCE src/version.cpp
  APPEND
  PROPERTY COMPILE_DEFINITIONS
  KHEPRI_VERSION_MAJOR=${KHEPRI_VERSION_MAJOR}
  KHEPRI_VERSION_MINOR=${KHEPRI_VERSION_MINOR}
  KHEPRI_VERSION_PATCH=${KHEPRI_VERSION_PATCH}
  KHEPRI_VERSION_STRING="${KHEPRI_VERSION_MAJOR}.${KHEPRI_VERSION_MINOR}.${KHEPRI_VERSION_PATCH}"
  KHEPRI_VERSION_COMMIT="${KHEPRI_VERSION_COMMIT}"
  KHEPRI_VERSION_CLEAN=${KHEPRI_VERSION_CLEAN}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    fmt::fmt
    freetype
    glfw
    gsl::gsl-lite
    diligent-core::diligent-core
)

if(MSVC)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      _CRT_SECURE_NO_WARNINGS
    PUBLIC
      PLATFORM_WIN32
  )

  target_link_libraries(${PROJECT_NAME}
    PRIVATE
      dxgi
      d3d11
      d3dcompiler
  )
                
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
      $<$<CONFIG:Debug>:DbgHelp>
  )
endif()

add_library(khepri::khepri ALIAS Khepri)

add_subdirectory(tools)

#
# Tests
#
include(CTest)
if (BUILD_TESTING)
    find_package(GTest REQUIRED)

    include(GoogleTest)

    add_executable(${PROJECT_NAME}Tests
        tests/cubic_spline_test.cpp
        tests/interpolator_test.cpp
        tests/matrix_test.cpp
        tests/polynomial_test.cpp
        tests/quaternion_test.cpp
    )

    target_link_Libraries(${PROJECT_NAME}Tests
        PRIVATE
            ${PROJECT_NAME}
            GTest::gmock_main
    )

    gtest_discover_tests(${PROJECT_NAME}Tests)
endif()

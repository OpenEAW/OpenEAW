cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(OpenEAW CXX)

# External libraries
find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/version.cpp
)

set_property(
  SOURCE src/version.cpp
  APPEND
  PROPERTY COMPILE_DEFINITIONS
  OPENEAW_VERSION_MAJOR=${OPENEAW_VERSION_MAJOR}
  OPENEAW_VERSION_MINOR=${OPENEAW_VERSION_MINOR}
  OPENEAW_VERSION_PATCH=${OPENEAW_VERSION_PATCH}
  OPENEAW_VERSION_STRING="${OPENEAW_VERSION_MAJOR}.${OPENEAW_VERSION_MINOR}.${OPENEAW_VERSION_PATCH}"
  OPENEAW_VERSION_COMMIT="${OPENEAW_VERSION_COMMIT}"
  OPENEAW_VERSION_CLEAN=${OPENEAW_VERSION_CLEAN}
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME}
    cxxopts::cxxopts
    fmt::fmt
    openglyph::openglyph
    khepri::khepri
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
)

add_subdirectory(khepri)
add_subdirectory(openglyph)

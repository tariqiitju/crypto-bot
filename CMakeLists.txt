cmake_minimum_required(VERSION 2.8.12)
project(restapp)

add_definitions("-std=c++17")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


file(GLOB SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.cpp
  ${PROJECT_SOURCE_DIR}/src/stream/*.cpp
  ${PROJECT_SOURCE_DIR}/tests/*.cpp
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
)


# add_library(acaccess SHARED ${SOURCES})
add_executable(restapp ${SOURCES})

target_link_libraries(restapp ${CONAN_LIBS})


